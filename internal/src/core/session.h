#ifndef SRC_SESSION_H_
#define SRC_SESSION_H_

#include "core/webrtc_transport.h"

class Session : public sigslot::has_slots<> {
 public:
  WebrtcTransport* CreateWebrtcTransport(const std::string& direction,
                                         const std::string& ip,
                                         const int port);

  void OnSenderPacket(Sender*, webrtc::RtpPacketReceived&);

  std::map<std::string, WebrtcTransport*> transports;
};

#endif /* SRC_SESSION_H_ */
