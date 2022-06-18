#ifndef SRC_RTP_DEMUXER_H_
#define SRC_RTP_DEMUXER_H_

#include <modules/rtp_rtcp/source/rtp_packet_received.h>
#include <rtc_base/containers/flat_map.h>

#include "common/logger.h"
#include "sender.h"

class RtpDemuxer {
  DECLARE_LOGGER();

 public:
  void AddSender(Sender* sender);
  void RemoveSender(Sender* sender);
  Sender* ResolveSender(const webrtc::RtpPacketReceived& packet);
  Sender* ResolveSenderByMid(const std::string& mid, uint32_t ssrc);
  void AddSsrcSinkBinding(uint32_t ssrc, Sender* sender);

  webrtc::flat_map<uint32_t, Sender*> sender_by_ssrc_;
  webrtc::flat_map<std::string, Sender*> sender_by_mid_;
  webrtc::flat_map<std::string, Sender*> sender_by_rid_;
};

#endif /* SRC_RTP_DEMUXER_H_ */
