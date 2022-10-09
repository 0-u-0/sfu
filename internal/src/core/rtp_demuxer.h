#ifndef SRC_RTP_DEMUXER_H_
#define SRC_RTP_DEMUXER_H_

#include <modules/rtp_rtcp/source/rtp_packet_received.h>
#include <rtc_base/containers/flat_map.h>

#include "common/logger.h"
#include "core/producer.h"

class RtpDemuxer {
  DECLARE_LOGGER();

 public:
  void AddSender(Producer* sender);
  void RemoveSender(Producer* sender);
  Producer* ResolveSender(const webrtc::RtpPacketReceived& packet);
  Producer* ResolveSenderByMid(const std::string& mid, uint32_t ssrc);
  void AddSsrcSinkBinding(uint32_t ssrc, Producer* sender);

  // TODO(CC): better using std::unordered_map
  webrtc::flat_map<uint32_t, Producer*> sender_by_ssrc_;
  webrtc::flat_map<std::string, Producer*> sender_by_mid_;
  webrtc::flat_map<std::string, Producer*> sender_by_rid_;
};

#endif /* SRC_RTP_DEMUXER_H_ */
