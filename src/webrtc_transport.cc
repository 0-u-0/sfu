
#include "webrtc_transport.h"

#include <iostream>
#include <type_traits>

#include <api/transport/stun.h>
#include <modules/rtp_rtcp/source/rtp_packet_received.h>
#include <rtc_base/helpers.h>
#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "receiver.h"
#include "rtp_demuxer.h"
#include "tools.h"

using namespace cricket;

WebrtcTransport::WebrtcTransport(const std::string& direction,
                                 const std::string& ip,
                                 const int port)
    : id_(rtc::CreateRandomUuid()) {
  direction_ = direction;
  thread_ = rtc::Thread::CreateWithSocketServer();
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

    rtp_header_extensions_.RegisterByType(9, webrtc::kRtpExtensionMid);
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

// network ->
void WebrtcTransport::OnPacket(rtc::CopyOnWriteBuffer& buffer) {
  webrtc::RtpPacketReceived packet;

  if (!packet.Parse(buffer))
    return;

  packet.IdentifyExtensions(rtp_header_extensions_);

  Sender* sender = rtp_demuxer_->ResolveSender(packet);
  if (sender != nullptr) {
    RTC_LOG(INFO) << "found sender";
    sender->OnRtpPacket(packet);
  }

  // RTC_LOG(INFO) << "packet: " << packet.Ssrc();
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
Sender* WebrtcTransport::CreateSender(json& codec) {
  auto* sender = new Sender(codec);
  this->mapSender[sender->id_] = sender;
  this->mapSenderReceiver[sender];
  sender->SignalReadPacket.connect(this, &WebrtcTransport::OnSenderPacket);
  rtp_demuxer_->AddSender(sender);
  return sender;
}

Sender* WebrtcTransport::GetSender(uint32_t ssrc,
                                   std::string mid,
                                   std::string rid) {}

void WebrtcTransport::OnSenderPacket(Sender* sender,
                                     webrtc::RtpPacketReceived& packet) {
  RTC_LOG(INFO) << "sender packet: " << sender->id_;

  auto& receivers = this->mapSenderReceiver.at(sender);

  for (auto* receiver : receivers) {
    receiver->OnSenderPacket(sender, packet);
  }

  // SignalReadPacket(sender, packet);
}

Receiver* WebrtcTransport::CreateReceiver() {
  auto receiver = new Receiver();
  receiver->SignalReadPacket.connect(this, &WebrtcTransport::OnReceiverPacket);
  return receiver;
}

void WebrtcTransport::OnReceiverPacket(Receiver* receiver,
                                       webrtc::RtpPacketReceived& packet) {
  // RTC_LOG(INFO) << "receiver packet: ";

  // SendPacket(const char *data, size_t size, const int64_t timestamp)
  // SignalReadPacket(sender, packet);
}

void WebrtcTransport::AddReceiverToSender(std::string senderId,
                                          Receiver* receiver) {
  RTC_LOG(INFO) << "add receiver " << receiver->id_ << " to " << senderId;

  auto* sender = this->mapSender[senderId];
  auto& receivers = this->mapSenderReceiver[sender];
  receivers.insert(receiver);
}
