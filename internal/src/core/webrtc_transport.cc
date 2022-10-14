
#include "webrtc_transport.h"

#include <iostream>
#include <type_traits>

#include "api/transport/stun.h"
#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "modules/rtp_rtcp/source/rtcp_packet/bye.h"
#include "modules/rtp_rtcp/source/rtcp_packet/common_header.h"
#include "modules/rtp_rtcp/source/rtcp_packet/extended_reports.h"
#include "modules/rtp_rtcp/source/rtcp_packet/fir.h"
#include "modules/rtp_rtcp/source/rtcp_packet/nack.h"
#include "modules/rtp_rtcp/source/rtcp_packet/pli.h"
#include "modules/rtp_rtcp/source/rtcp_packet/psfb.h"
#include "modules/rtp_rtcp/source/rtcp_packet/rapid_resync_request.h"
#include "modules/rtp_rtcp/source/rtcp_packet/receiver_report.h"
#include "modules/rtp_rtcp/source/rtcp_packet/rtpfb.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sdes.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sender_report.h"
#include "modules/rtp_rtcp/source/rtcp_packet/tmmbn.h"
#include "modules/rtp_rtcp/source/rtcp_packet/tmmbr.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"

#include "modules/rtp_rtcp/source/rtp_packet_received.h"
#include "modules/rtp_rtcp/source/rtp_util.h"

#include "rtc_base/helpers.h"
#include "rtc_base/task_utils/to_queued_task.h"

#include "common/tools.h"
#include "core/consumer.h"
#include "core/rtp_demuxer.h"

DEFINE_LOGGER(WebrtcTransport, "WebrtcTransport")

WebrtcTransport::WebrtcTransport(const std::string& direction,
                                 const std::string& ip,
                                 const int port)
    : id_(rtc::CreateRandomUuid()), ip_(ip), port_(port) {
  direction_ = direction;
  thread_ = rtc::Thread::Create();
  thread_->Start();

  ice_transport_ = thread_->Invoke<IceTransport*>(
      RTC_FROM_HERE, [ip, port]() { return new IceTransport(ip, port); });
  dtls_transport_ = thread_->Invoke<DtlsTransport*>(RTC_FROM_HERE, [this]() {
    return new DtlsTransport(ice_transport_, webrtc::CryptoOptions(),
                             rtc::SSL_PROTOCOL_DTLS_12);
  });

  srtp_transport_ = thread_->Invoke<SrtpTransport*>(RTC_FROM_HERE, [this]() {
    auto srtp = new SrtpTransport();
    srtp->packet_callback_list_.AddReceiver(
        // TODO(CC): remove copy
        this, [this](rtc::CopyOnWriteBuffer packet) {
          this->packet_callback_list_.Send(std::move(packet));
        });
    srtp->SetDtlsTransport(dtls_transport_);
    return srtp;
  });

  rtp_demuxer_ = new RtpDemuxer();
}

void WebrtcTransport::Init() {
  thread_->PostTask(webrtc::ToQueuedTask([this]() {
    bool is_client = true;
    if (direction_ == "recvonly") {
      is_client = false;
    }
    packet_callback_list_.AddReceiver(
        [this](rtc::CopyOnWriteBuffer packet) { OnPacket(packet); });

    rtp_header_extensions_.RegisterByType(4, webrtc::kRtpExtensionMid);
    rtp_header_extensions_.RegisterByType(10, webrtc::kRtpExtensionRtpStreamId);
    rtp_header_extensions_.RegisterByType(
        11, webrtc::kRtpExtensionRepairedRtpStreamId);

    dtls_transport_->Init(is_client);
    ice_transport_->Init();
  }));
}

// -> network
void WebrtcTransport::SendPacket(const char* data,
                                 size_t size,
                                 const int64_t timestamp) {
  auto packet = rtc::CopyOnWriteBuffer(data, size);

  thread_->PostTask(
      webrtc::ToQueuedTask([this, packet_copy = std::move(packet)]() {
        rtc::PacketOptions packet_options;

        srtp_transport_->SendRtpPacket(
            reinterpret_cast<const char*>(packet_copy.data()),
            packet_copy.size(), packet_options);
      }));
}

void WebrtcTransport::SendPacket(webrtc::RtpPacketReceived& received_packet) {
  auto packet = rtc::CopyOnWriteBuffer(received_packet.Buffer());

  thread_->PostTask(
      webrtc::ToQueuedTask([this, packet_copy = std::move(packet)]() {
        rtc::PacketOptions packet_options;

        srtp_transport_->SendRtpPacket(
            reinterpret_cast<const char*>(packet_copy.data()),
            packet_copy.size(), packet_options);
      }));
}

// network ->
void WebrtcTransport::OnPacket(rtc::CopyOnWriteBuffer& buffer) {
  webrtc::RtpPacketReceived packet;

  if (!packet.Parse(buffer.cdata(), buffer.size()))
    return;

  if (webrtc::IsRtcpPacket(buffer)) {
    DLOG("rtcp");
    HandleRtcp(buffer);
  } else if (webrtc::IsRtpPacket(buffer)) {
    packet.IdentifyExtensions(rtp_header_extensions_);
    // TODO(CC):check type
    packet.set_payload_type_frequency(webrtc::kVideoPayloadTypeFrequency);

    Producer* producer = rtp_demuxer_->ResolveSender(packet);
    if (producer != nullptr) {
      DLOG("found producer");
      producer->OnRtpPacket(packet);
    }
  }

  // RTC_LOG(INFO) << "packet: " << packet.Ssrc();
}

void WebrtcTransport::HandleRtcp(rtc::ArrayView<const uint8_t> packet) {
  // https://github.com/versatica/mediasoup/blob/v3/doc/RTCP.md
  webrtc::rtcp::CommonHeader rtcp_block;

  size_t num_skipped_packets_ = 0;

  // If a sender report is received but no DLRR, we need to reset the
  // roundTripTime stat according to the standard, see
  // https://www.w3.org/TR/webrtc-stats/#dom-rtcremoteoutboundrtpstreamstats-roundtriptime
  struct RtcpReceivedBlock {
    bool sender_report = false;
    bool dlrr = false;
  };
  // For each remote SSRC we store if we've received a sender report or a DLRR
  // block.
  webrtc::flat_map<uint32_t, RtcpReceivedBlock> received_blocks;
  for (const uint8_t* next_block = packet.begin(); next_block != packet.end();
       next_block = rtcp_block.NextPacket()) {
    ptrdiff_t remaining_blocks_size = packet.end() - next_block;
    RTC_DCHECK_GT(remaining_blocks_size, 0);
    if (!rtcp_block.Parse(next_block, remaining_blocks_size)) {
      if (next_block == packet.begin()) {
        // Failed to parse 1st header, nothing was extracted from this packet.
        WLOG("Incoming invalid RTCP packet");
        return;
      }
      ++num_skipped_packets_;
      break;
    }

    // if (packet_type_counter_.first_packet_time_ms == -1)
    //   packet_type_counter_.first_packet_time_ms =
    //   clock_->TimeInMilliseconds();

    ILOG("rtcp type: {}", rtcp_block.type());

    switch (rtcp_block.type()) {
      case webrtc::rtcp::SenderReport::kPacketType:
        ILOG("rtcp type: SenderReport");
        // HandleSenderReport(rtcp_block, packet_information);
        // received_blocks[packet_information->remote_ssrc].sender_report =
        // true;
        break;
      case webrtc::rtcp::ReceiverReport::kPacketType:
        ILOG("rtcp type: ReceiverReport");
        // HandleReceiverReport(rtcp_block, packet_information);
        break;
      case webrtc::rtcp::Sdes::kPacketType:
        // nothing to do
        ILOG("rtcp type: Sdes");
        // HandleSdes(rtcp_block, packet_information);
        break;
      case webrtc::rtcp::ExtendedReports::kPacketType: {
        // bool contains_dlrr = false;
        // uint32_t ssrc = 0;
        // HandleXr(rtcp_block, packet_information, contains_dlrr, ssrc);
        // if (contains_dlrr) {
        //   received_blocks[ssrc].dlrr = true;
        // }
        break;
      }
      case webrtc::rtcp::Bye::kPacketType:
        // HandleBye(rtcp_block);
        break;
      case webrtc::rtcp::App::kPacketType:
        // HandleApp(rtcp_block, packet_information);
        break;
      case webrtc::rtcp::Rtpfb::kPacketType:
        switch (rtcp_block.fmt()) {
          case webrtc::rtcp::Nack::kFeedbackMessageType:
            // HandleNack(rtcp_block, packet_information);
            break;
          case webrtc::rtcp::Tmmbr::kFeedbackMessageType:
            // HandleTmmbr(rtcp_block, packet_information);
            break;
          case webrtc::rtcp::Tmmbn::kFeedbackMessageType:
            // HandleTmmbn(rtcp_block, packet_information);
            break;
          case webrtc::rtcp::RapidResyncRequest::kFeedbackMessageType:
            // HandleSrReq(rtcp_block, packet_information);
            break;
          case webrtc::rtcp::TransportFeedback::kFeedbackMessageType:
            // HandleTransportFeedback(rtcp_block, packet_information);
            break;
          default:
            ++num_skipped_packets_;
            break;
        }
        break;
      case webrtc::rtcp::Psfb::kPacketType:
        switch (rtcp_block.fmt()) {
          case webrtc::rtcp::Pli::kFeedbackMessageType:
            // HandlePli(rtcp_block, packet_information);
            break;
          case webrtc::rtcp::Fir::kFeedbackMessageType:
            // HandleFir/(rtcp_block, packet_information);
            break;
          case webrtc::rtcp::Psfb::kAfbMessageType:
            // HandlePsfbApp(rtcp_block, packet_information);
            break;
          default:
            ++num_skipped_packets_;
            break;
        }
        break;
      default:
        ++num_skipped_packets_;
        break;
    }
  }
}

bool WebrtcTransport::SetLocalCertificate(
    const rtc::scoped_refptr<rtc::RTCCertificate>& certificate) {
  return thread_->Invoke<bool>(RTC_FROM_HERE, [this, certificate]() {
    return dtls_transport_->SetLocalCertificate(certificate);
  });
}

bool WebrtcTransport::SetRemoteFingerprint(const std::string& algorithm,
                                           const std::string& fingerprint) {
  return thread_->Invoke<bool>(RTC_FROM_HERE, [this, algorithm, fingerprint]() {
    return dtls_transport_->SetRemoteFingerprint(algorithm, fingerprint);
  });
}

// TODO(CC): move to network thread
Producer* WebrtcTransport::Produce(std::string& type,
                                   RtpParameters& parameter) {
  MediaType kind = StringToMediaType(type);
  // TODO(CC): error
  if (kind == MediaType::ANY) {
    return nullptr;
  }

  auto* sender = new Producer(kind, parameter);
  this->mapSender[sender->id_] = sender;
  this->mapSenderReceiver[sender];
  sender->SignalReadPacket.connect(this, &WebrtcTransport::OnSenderPacket);
  rtp_demuxer_->AddSender(sender);
  return sender;
}

Producer* WebrtcTransport::GetProducer(uint32_t ssrc,
                                       std::string mid,
                                       std::string rid) {
  return nullptr;
}

void WebrtcTransport::OnSenderPacket(Producer* sender,
                                     webrtc::RtpPacketReceived& packet) {
  DLOG("sender packet: {}", sender->id_);

  auto& receivers = this->mapSenderReceiver.at(sender);

  for (auto* receiver : receivers) {
    receiver->OnSenderPacket(sender, packet);
  }

  // SignalReadPacket(sender, packet);
}

Consumer* WebrtcTransport::CreateReceiver(MediaType kind,
                                          RtpParameters& sender_parameter) {
  auto receiver = new Consumer(kind, sender_parameter);
  receiver->SignalReadPacket.connect(this, &WebrtcTransport::OnReceiverPacket);
  return receiver;
}

void WebrtcTransport::OnReceiverPacket(Consumer* receiver,
                                       webrtc::RtpPacketReceived& packet) {
  // RTC_LOG(INFO) << "receiver packet: ";

  SendPacket(packet);
  // SendPacket(const char *data, size_t size, const int64_t timestamp)
  // SignalReadPacket(sender, packet);
}

void WebrtcTransport::AddReceiverToSender(std::string senderId,
                                          Consumer* receiver) {
  ILOG("Add receiver {} to {}", receiver->id_, senderId);

  auto* sender = this->mapSender[senderId];
  auto& receivers = this->mapSenderReceiver[sender];
  receivers.insert(receiver);
}
