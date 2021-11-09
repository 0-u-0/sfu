#ifndef SRC_RECEIVER_H_
#define SRC_RECEIVER_H_

#include <string>

#include "sender.h"

class Receiver : public sigslot::has_slots<> {
 public:
  Receiver();
  void OnSenderPacket(Sender*, webrtc::RtpPacketReceived&);

  sigslot::signal2<Receiver*, webrtc::RtpPacketReceived&> SignalReadPacket;

  const std::string id_;
};

#endif /* SRC_RECEIVER_H_ */
