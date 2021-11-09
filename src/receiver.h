#ifndef SRC_RECEIVER_H_
#define SRC_RECEIVER_H_

#include <string>

#include "sender.h"

class Receiver {
 public:
  Receiver();
  void OnSenderPacket(Sender*, webrtc::RtpPacketReceived&);

  const std::string id_;
};

#endif /* SRC_RECEIVER_H_ */
