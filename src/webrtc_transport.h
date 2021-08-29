#ifndef SRC_WEBRTC_TRANSPORT_H_
#define SRC_WEBRTC_TRANSPORT_H_

#include <string>

#include "udp_transport.h"

class WebrtcTransport {
 public:
  WebrtcTransport(const std::string& ip, const int port);

  void Receive(const char* data, size_t size, const rtc::SocketAddress& addr);
  void Init();

  //variable
  std::string local_ufrag_;
  std::string local_password_;
  UdpTransport *udp_transport;
};

#endif /* SRC_WEBRTC_TRANSPORT_H_ */
