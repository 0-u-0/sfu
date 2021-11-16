
#include "receiver.h"

#include "rtc_base/helpers.h"
#include "rtc_base/logging.h"

Receiver::Receiver(RtpParameters& parameter) : id_(rtc::CreateRandomUuid()) {
  rtp_parameter_ = parameter;
}

void Receiver::OnSenderPacket(Sender*, webrtc::RtpPacketReceived& packet) {
  RTC_LOG(INFO) << "receiver on packet";

  SignalReadPacket(this, packet);
}
