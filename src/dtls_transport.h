#ifndef SRC_DTLS_TRANSPORT_H_
#define SRC_DTLS_TRANSPORT_H_

#include "rtc_base/rtc_certificate.h"
#include "rtc_base/ssl_stream_adapter.h"
#include "rtc_base/strings/string_builder.h"

enum DtlsTransportState {
  // Haven't started negotiating.
  DTLS_TRANSPORT_NEW = 0,
  // Have started negotiating.
  DTLS_TRANSPORT_CONNECTING,
  // Negotiated, and has a secure connection.
  DTLS_TRANSPORT_CONNECTED,
  // Transport is closed.
  DTLS_TRANSPORT_CLOSED,
  // Failed due to some error in the handshake process.
  DTLS_TRANSPORT_FAILED,
};

/**
active : client
passive : server
*/
class DtlsTransport {
 public:
  DtlsTransport();

  static rtc::scoped_refptr<rtc::RTCCertificate> certificate_;
  static const rtc::scoped_refptr<rtc::RTCCertificate>& certificate();

  void OnPacket(const char* data, size_t size);
  bool HandleDtlsPacket(const char* data, size_t size);
  bool GetDtlsRole(rtc::SSLRole* role) const;
  bool SetDtlsRole(rtc::SSLRole role);
  bool SetupDtls();
  
  DtlsTransportState dtls_state() const;
  std::string ToString() const {
    const absl::string_view RECEIVING_ABBREV[2] = {"_", "R"};
    const absl::string_view WRITABLE_ABBREV[2] = {"_", "W"};
    rtc::StringBuilder sb;
    sb << "DtlsTransport[";
    return sb.Release();
  }

  absl::optional<rtc::SSLRole> dtls_role_;
  const std::vector<int> srtp_ciphers_;  // SRTP ciphers to use with DTLS.
  std::unique_ptr<rtc::SSLStreamAdapter> dtls_;  // The DTLS stream
  rtc::scoped_refptr<rtc::RTCCertificate> local_certificate_;
  DtlsTransportState dtls_state_ = DTLS_TRANSPORT_NEW;
  // Cached DTLS ClientHello packet that was received before we started the
  // DTLS handshake. This could happen if the hello was received before the
  // ice transport became writable, or before a remote fingerprint was received.
  rtc::Buffer cached_client_hello_;


};

#endif /* SRC_DTLS_TRANSPORT_H_ */
