#ifndef SRC_SRTP_TRANSPORT_H_
#define SRC_SRTP_TRANSPORT_H_

#include <api/crypto_params.h>
#include <pc/srtp_session.h>
#include <rtc_base/callback_list.h>
#include <rtc_base/copy_on_write_buffer.h>

#include "common/logger.h"
#include "core/dtls_transport.h"

class SrtpTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  SrtpTransport();

  void SetDtlsTransport(DtlsTransport* dtls_transport);
  void SetupRtpDtlsSrtp();
  void OnDtlsState(DtlsTransport* transport, const DtlsTransportState state);
  bool ExtractParams(DtlsTransport* dtls_transport,
                     int* selected_crypto_suite,
                     rtc::ZeroOnFreeBuffer<unsigned char>* send_key,
                     rtc::ZeroOnFreeBuffer<unsigned char>* recv_key);
  bool SetRtpParams(int send_cs,
                    const uint8_t* send_key,
                    int send_key_len,
                    const std::vector<int>& send_extension_ids,
                    int recv_cs,
                    const uint8_t* recv_key,
                    int recv_key_len,
                    const std::vector<int>& recv_extension_ids);

  void OnRtpPacketReceived(rtc::CopyOnWriteBuffer packet,
                           int64_t packet_time_us);

  void OnPacketReceived(rtc::CopyOnWriteBuffer packet, int64_t packet_time_us);

  void OnReadPacket(const char* data,
                    size_t len,
                    const int64_t packet_time_us,
                    int flags);
  bool UnprotectRtp(void* p, int in_len, int* out_len);
  bool UnprotectRtcp(void* p, int in_len, int* out_len);

  void ResetParams();
  void CreateSrtpSessions();

  bool SendRtpPacket(const char* data,
                     size_t size,
                     const rtc::PacketOptions& options);
  bool ProtectRtp(void* p, int in_len, int max_len, int* out_len);
  bool IsSrtpActive() const;

  DtlsTransport* dtls_transport_;
  absl::optional<cricket::CryptoParams> send_params_;
  absl::optional<cricket::CryptoParams> recv_params_;
  absl::optional<int> send_cipher_suite_;
  absl::optional<int> recv_cipher_suite_;
  rtc::ZeroOnFreeBuffer<uint8_t> send_key_;
  rtc::ZeroOnFreeBuffer<uint8_t> recv_key_;
  std::unique_ptr<cricket::SrtpSession> send_session_;
  std::unique_ptr<cricket::SrtpSession> recv_session_;
  // The encrypted header extension IDs.
  absl::optional<std::vector<int>> send_extension_ids_;
  absl::optional<std::vector<int>> recv_extension_ids_;
  webrtc::CallbackList<rtc::CopyOnWriteBuffer> packet_callback_list_;

  int decryption_failure_count_ = 0;
};

#endif /* SRC_SRTP_TRANSPORT_H_ */
