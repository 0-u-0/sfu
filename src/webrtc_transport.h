#ifndef SRC_WEBRTC_TRANSPORT_H_
#define SRC_WEBRTC_TRANSPORT_H_

#include <string>

#include "dtls_transport.h"
#include "ice_transport.h"

class WebrtcTransport {
 public:
  WebrtcTransport(const std::string& ip, const int port);

  void Init();
  bool SetLocalCertificate(
      const rtc::scoped_refptr<rtc::RTCCertificate>& certificate);
  rtc::scoped_refptr<rtc::RTCCertificate> GetLocalCertificate() const;

  bool SetRemoteFingerprint(const std::string& algorithm,
                            const std::string& fingerprint);

  DtlsTransport* dtls_transport_;
  IceTransport* ice_transport_;
};

#endif /* SRC_WEBRTC_TRANSPORT_H_ */
