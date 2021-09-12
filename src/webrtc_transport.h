#ifndef SRC_WEBRTC_TRANSPORT_H_
#define SRC_WEBRTC_TRANSPORT_H_

#include <string>

#include "ice_transport.h"
#include "dtls_transport.h"

class WebrtcTransport {
 public:
  WebrtcTransport(const std::string& ip, const int port);

  void Init();

  DtlsTransport* dtls_transport_;
  IceTransport *ice_transport_;
};

#endif /* SRC_WEBRTC_TRANSPORT_H_ */
