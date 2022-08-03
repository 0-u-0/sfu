#ifndef SRC_DTLS_TRANSPORT_H_
#define SRC_DTLS_TRANSPORT_H_

#include "api/crypto/crypto_options.h"
#include "api/sequence_checker.h"
#include "rtc_base/buffer_queue.h"
#include "rtc_base/callback_list.h"
#include "rtc_base/rtc_certificate.h"
#include "rtc_base/ssl_stream_adapter.h"
#include "rtc_base/strings/string_builder.h"
#include "rtc_base/system/no_unique_address.h"

#include "common/logger.h"
#include "core/ice_transport.h"

// A bridge between a packet-oriented/transport-type interface on
// the bottom and a StreamInterface on the top.
class StreamInterfaceChannel : public rtc::StreamInterface {
 public:
  explicit StreamInterfaceChannel(IceTransport* ice_transport);

  // Push in a packet; this gets pulled out from Read().
  bool OnPacketReceived(const char* data, size_t size);

  // Implementations of StreamInterface
  rtc::StreamState GetState() const override;
  void Close() override;
  rtc::StreamResult Read(void* buffer,
                         size_t buffer_len,
                         size_t* read,
                         int* error) override;
  rtc::StreamResult Write(const void* data,
                          size_t data_len,
                          size_t* written,
                          int* error) override;

 private:
  RTC_NO_UNIQUE_ADDRESS webrtc::SequenceChecker sequence_checker_;
  IceTransport* const ice_transport_;  // owned by DtlsTransport
  rtc::StreamState state_ RTC_GUARDED_BY(sequence_checker_);
  rtc::BufferQueue packets_ RTC_GUARDED_BY(sequence_checker_);

  RTC_DISALLOW_COPY_AND_ASSIGN(StreamInterfaceChannel);
};

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
class DtlsTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  DtlsTransport(
      IceTransport* ice_transport,
      const webrtc::CryptoOptions& crypto_options,
      rtc::SSLProtocolVersion max_version = rtc::SSL_PROTOCOL_DTLS_12);

  void Init(bool is_client);

  static rtc::scoped_refptr<rtc::RTCCertificate> certificate_;
  static const rtc::scoped_refptr<rtc::RTCCertificate>& certificate();

  bool SetLocalCertificate(
      const rtc::scoped_refptr<rtc::RTCCertificate>& certificate);
  rtc::scoped_refptr<rtc::RTCCertificate> GetLocalCertificate() const;

  bool SetRemoteFingerprint(const std::string& algorithm,
                            const std::string& fingerprint);
  bool SetRemoteFingerprint(const std::string& digest_alg,
                            const uint8_t* digest,
                            size_t digest_len);

  void OnPacket(const char* data, size_t size, const int64_t packet_time_us);
  void OnIceTransportState(IceTransportState state);
  bool HandleDtlsPacket(const char* data, size_t size);
  bool GetDtlsRole(rtc::SSLRole* role) const;
  bool SetDtlsRole(rtc::SSLRole role);
  bool SetupDtls();
  void OnDtlsHandshakeError(rtc::SSLHandshakeError error);
  void OnDtlsEvent(rtc::StreamInterface* stream_, int sig, int err);
  void MaybeStartDtls();
  void ConfigureHandshakeTimeout();

  void set_dtls_state(DtlsTransportState state);
  void SendDtlsState(DtlsTransport* transport, DtlsTransportState state);
  bool GetSrtpCryptoSuite(int* cipher);
  bool ExportKeyingMaterial(const std::string& label,
                            const uint8_t* context,
                            size_t context_len,
                            bool use_context,
                            uint8_t* result,
                            size_t result_len);

  DtlsTransportState dtls_state() const;

  bool dtls_active_ = false;
  IceTransport* ice_transport_;
  absl::optional<rtc::SSLRole> dtls_role_;
  const std::vector<int> srtp_ciphers_;  // SRTP ciphers to use with DTLS.
  std::unique_ptr<rtc::SSLStreamAdapter> dtls_;  // The DTLS stream
  rtc::scoped_refptr<rtc::RTCCertificate> local_certificate_;
  DtlsTransportState dtls_state_ = DTLS_TRANSPORT_NEW;
  // Cached DTLS ClientHello packet that was received before we started the
  // DTLS handshake. This could happen if the hello was received before the
  // ice transport became writable, or before a remote fingerprint was received.
  rtc::Buffer cached_client_hello_;
  const rtc::SSLProtocolVersion ssl_max_version_;
  rtc::Buffer remote_fingerprint_value_;
  std::string remote_fingerprint_algorithm_;
  StreamInterfaceChannel*
      downward_;  // Wrapper for ice_transport_, owned by dtls_.
  webrtc::SequenceChecker thread_checker_;
  webrtc::CallbackList<DtlsTransport*, const DtlsTransportState>
      dtls_state_callback_list_;
  // Signalled each time a packet is received on this channel.
  sigslot::signal4<const char*, size_t, const int64_t, int> on_packet_;
};

#endif /* SRC_DTLS_TRANSPORT_H_ */
