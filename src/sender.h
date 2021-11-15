#ifndef SRC_SENDER_H_
#define SRC_SENDER_H_

#include <json.hpp>

#include <modules/rtp_rtcp/source/rtp_packet_received.h>
#include <rtc_base/third_party/sigslot/sigslot.h>

#include "media_types.h"
#include "ortc/rtp_parameters.h"

using json = nlohmann::json;

class Sender : public sigslot::has_slots<> {
 public:
  Sender(RtpParameters& parameter);
  const std::string id_;

  std::string mid_;
  uint32_t ssrc_;
  uint32_t rtx_ssrc_;

  MediaType kind_;

  RtpParameters rtp_parameter_;

  sigslot::signal2<Sender*, webrtc::RtpPacketReceived&> SignalReadPacket;

  void OnRtpPacket(webrtc::RtpPacketReceived& packet);
};

#endif /* SRC_SENDER_H_ */
