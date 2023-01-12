#ifndef SRC_SENDER_H_
#define SRC_SENDER_H_

#include <modules/rtp_rtcp/source/rtcp_packet/sender_report.h>
#include <modules/rtp_rtcp/source/rtp_packet_received.h>
#include <rtc_base/third_party/sigslot/sigslot.h>

#include "common/logger.h"
#include "core/media_types.h"
#include "ortc/rtp_parameters.h"

#include "third_party/nlohmann/json.hpp"

using json = nlohmann::json;

class Producer : public sigslot::has_slots<> {
 public:
  DECLARE_LOGGER();

  Producer(MediaType kind, RtpParameters& parameter);
  void OnRtpPacket(webrtc::RtpPacketReceived& packet);

  void HandleSenderReport(webrtc::rtcp::SenderReport& sender_report);

  const std::string id_;

  std::string mid_;
  uint32_t ssrc_;
  uint32_t rtx_ssrc_;

  MediaType kind_;

  RtpParameters rtp_parameter_;

  sigslot::signal2<Producer*, webrtc::RtpPacketReceived&> SignalReadPacket;
};

#endif /* SRC_SENDER_H_ */
