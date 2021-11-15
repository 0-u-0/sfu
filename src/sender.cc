
#include "sender.h"

#include <rtc_base/helpers.h>

// TODO(CC): move codec parse to A class
Sender::Sender(webrtc::RtpParameters& parameter)
    : id_(rtc::CreateRandomUuid()) {
  this->ssrc_ = parameter.encodings[0].ssrc.value();
  this->mid_ = parameter.mid;
  // if (codec["rtx"].is_object()) {
  //   this->rtx_ssrc_ = codec["rtx"]["ssrc"];
  // }
}

void Sender::OnRtpPacket(webrtc::RtpPacketReceived& packet) {
  SignalReadPacket(this, packet);
}
