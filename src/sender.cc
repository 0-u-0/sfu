
#include "sender.h"

#include <rtc_base/helpers.h>

// TODO(CC): move codec parse to A class
Sender::Sender(json& encodings) : id_(rtc::CreateRandomUuid()) {
  this->ssrc_ = encodings[0]["ssrc"];
  this->mid_ = "0";
  // if (codec["rtx"].is_object()) {
  //   this->rtx_ssrc_ = codec["rtx"]["ssrc"];
  // }
}

void Sender::OnRtpPacket(webrtc::RtpPacketReceived& packet) {
  SignalReadPacket(this, packet);
}
