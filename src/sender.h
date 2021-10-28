#ifndef SRC_SENDER_H_
#define SRC_SENDER_H_

#include <json.hpp>

#include <modules/rtp_rtcp/source/rtp_packet_received.h>

#include "media_types.h"

using json = nlohmann::json;

class Sender {
 public:
  Sender(json& codec);
  const std::string id_;

  std::string mid_;
  uint32_t ssrc_;
  uint32_t rtx_ssrc_;

  MediaType kind_;

  void OnRtpPacket(const webrtc::RtpPacketReceived& packet);
};

#endif /* SRC_SENDER_H_ */
