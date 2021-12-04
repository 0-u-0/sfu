
#include "receiver.h"

#include "rtc_base/helpers.h"

DEFINE_LOGGER(Receiver, "Receiver");

Receiver::Receiver(RtpParameters& parameter) : id_(rtc::CreateRandomUuid()) {
  rtp_parameter_ = parameter;
  rtp_parameter_.encodings[0].ssrc = 11111111;
}

void Receiver::OnSenderPacket(Sender*, webrtc::RtpPacketReceived& packet) {
  ILOG("receiver on packet");
  packet.SetSsrc(rtp_parameter_.encodings[0].ssrc.value());
  SignalReadPacket(this, packet);
}
