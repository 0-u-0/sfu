#ifndef SRC_RECEIVER_H_
#define SRC_RECEIVER_H_

#include <string>

#include "sender.h"

class Receiver : public sigslot::has_slots<> {
 public:
  Receiver(RtpParameters& parameter);
  void OnSenderPacket(Sender*, webrtc::RtpPacketReceived&);

  sigslot::signal2<Receiver*, webrtc::RtpPacketReceived&> SignalReadPacket;

  const std::string id_;
  RtpParameters rtp_parameter_;
};

#endif /* SRC_RECEIVER_H_ */
