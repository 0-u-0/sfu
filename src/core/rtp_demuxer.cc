
#include "rtp_demuxer.h"
#include <cstddef>

#include "modules/rtp_rtcp/source/rtp_header_extensions.h"
#include "rtc_base/logging.h"
#include "sender.h"

void RtpDemuxer::AddSender(Sender* sender) {
  AddSsrcSinkBinding(sender->ssrc_, sender);
}

void RtpDemuxer::RemoveSender(Sender* sender) {}

Sender* RtpDemuxer::ResolveSender(const webrtc::RtpPacketReceived& packet) {
  std::string packet_mid, packet_rsid;
  bool has_mid = packet.GetExtension<webrtc::RtpMid>(&packet_mid);
  bool has_rsid = packet.GetExtension<webrtc::RtpStreamId>(&packet_rsid);

  uint32_t ssrc = packet.Ssrc();

  RTC_LOG(INFO) << "has_rsid: " << has_rsid;
  RTC_LOG(INFO) << "has_mid: " << has_mid;
  RTC_LOG(INFO) << "mid: " << packet_mid;

  Sender* sender = ResolveSenderByMid(packet_mid, ssrc);
  if (sender != nullptr) {
    return sender;
  }

  // We trust signaled SSRC more than payload type which is likely to conflict
  // between streams.
  const auto ssrc_sink_it = sender_by_ssrc_.find(ssrc);
  if (ssrc_sink_it != sender_by_ssrc_.end()) {
    return ssrc_sink_it->second;
  }

  return nullptr;
}

Sender* RtpDemuxer::ResolveSenderByMid(const std::string& mid, uint32_t ssrc) {
  const auto it = sender_by_mid_.find(mid);
  if (it != sender_by_mid_.end()) {
    Sender* sink = it->second;
    // AddSsrcSinkBinding(ssrc, sink);
    return sink;
  }
  return nullptr;
}

void RtpDemuxer::AddSsrcSinkBinding(uint32_t ssrc, Sender* sender) {
  // if (sink_by_ssrc_.size() >= kMaxSsrcBindings) {
  //   RTC_LOG(LS_WARNING) << "New SSRC=" << ssrc
  //                       << " sink binding ignored; limit of" <<
  //                       kMaxSsrcBindings
  //                       << " bindings has been reached.";
  //   return;
  // }

  auto result = sender_by_ssrc_.emplace(ssrc, sender);
  auto it = result.first;
  bool inserted = result.second;
  if (inserted) {
    RTC_LOG(LS_INFO) << "Added sink = " << sender
                     << " binding with SSRC=" << ssrc;
  } else if (it->second != sender) {
    RTC_LOG(LS_INFO) << "Updated sink = " << sender
                     << " binding with SSRC=" << ssrc;
    it->second = sender;
  }
}
