
#include "core/sender.h"

#include <rtc_base/helpers.h>

// TODO(CC): move codec parse to A class
Sender::Sender(RtpParameters& parameter) : id_(rtc::CreateRandomUuid()) {
  rtp_parameter_ = parameter;
  ssrc_ = parameter.encodings[0].ssrc.value();
  mid_ = parameter.mid;
  // if (codec["rtx"].is_object()) {
  //   this->rtx_ssrc_ = codec["rtx"]["ssrc"];
  // }
}

void Sender::OnRtpPacket(webrtc::RtpPacketReceived& packet) {
  SignalReadPacket(this, packet);
}
