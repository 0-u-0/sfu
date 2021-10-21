#ifndef SRC_WEBRTC_TRANSPORT_H_
#define SRC_WEBRTC_TRANSPORT_H_

#include <string>

#include <rtc_base/callback_list.h>
#include <rtc_base/thread.h>
#include <json.hpp>

#include "dtls_transport.h"
#include "ice_transport.h"
#include "sender.h"
#include "srtp_transport.h"

using json = nlohmann::json;

class WebrtcTransport : public sigslot::has_slots<> {
 public:
  WebrtcTransport(const std::string& direction,
                  const std::string& ip,
                  const int port);

  void Init();
  bool SetLocalCertificate(
      const rtc::scoped_refptr<rtc::RTCCertificate>& certificate);
  rtc::scoped_refptr<rtc::RTCCertificate> GetLocalCertificate() const;

  bool SetRemoteFingerprint(const std::string& algorithm,
                            const std::string& fingerprint);

  void OnPacket(const char* data, size_t size, const int64_t timestamp);

  Sender* CreateSender(json codec);

  std::unique_ptr<rtc::Thread> thread_;

  std::string direction_;
  DtlsTransport* dtls_transport_;
  IceTransport* ice_transport_;
  SrtpTransport* srtp_transport_;

  webrtc::CallbackList<rtc::CopyOnWriteBuffer> packet_callback_list_;
};

#endif /* SRC_WEBRTC_TRANSPORT_H_ */
