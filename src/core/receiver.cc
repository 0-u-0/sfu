
#include "receiver.h"

#include "rtc_base/helpers.h"

DEFINE_LOGGER(Receiver, "Receiver");

Receiver::Receiver(RtpParameters& parameter) : id_(rtc::CreateRandomUuid()) {
  rtp_parameter_ = parameter;
}

void Receiver::OnSenderPacket(Sender*, webrtc::RtpPacketReceived& packet) {
  ILOG("receiver on packet");

  SignalReadPacket(this, packet);
}
