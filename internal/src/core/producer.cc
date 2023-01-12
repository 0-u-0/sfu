
#include "core/producer.h"

#include <rtc_base/helpers.h>

DEFINE_LOGGER(Producer, "Producer");

// TODO(CC): move codec parse to A class
Producer::Producer(MediaType kind, RtpParameters& parameter)
    : id_(rtc::CreateRandomUuid()) {
  kind_ = kind;
  rtp_parameter_ = parameter;
  ssrc_ = parameter.encodings[0].ssrc.value();
  mid_ = parameter.mid;
  // if (codec["rtx"].is_object()) {
  //   this->rtx_ssrc_ = codec["rtx"]["ssrc"];
  // }
}

void Producer::OnRtpPacket(webrtc::RtpPacketReceived& packet) {
  /**
   *   Producer::ReceiveRtpPacket
   *
   *   1. get or create  rtpStream by packet
   *   2. send packet to rtpStream
   *   3. using rtpStream:  nack, rtx, keyFrame
   *   4. mangle packet, change payload, ssrc, etc.  (Producer::MangleRtpPacket)
   */
  SignalReadPacket(this, packet);
}

void Producer::HandleSenderReport(webrtc::rtcp::SenderReport& sender_report) {
  ILOG("HandleSenderReport");
}
