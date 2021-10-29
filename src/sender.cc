
#include "sender.h"

#include <rtc_base/helpers.h>

// TODO(CC): move codec parse to A class
Sender::Sender(json& codec) : id_(rtc::CreateRandomUuid()) {
  this->ssrc_ = codec["ssrc"];
  this->mid_ = codec["mid"];
  if (codec["rtx"].is_object()) {
    this->rtx_ssrc_ = codec["rtx"]["ssrc"];
  }
}

void Sender::OnRtpPacket(webrtc::RtpPacketReceived& packet) {
  SignalReadPacket(this, packet);
}
