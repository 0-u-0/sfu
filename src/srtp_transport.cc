
#include "srtp_transport.h"

#include <media/base/rtp_utils.h>
#include <rtc_base/logging.h>
#include <rtc_base/numerics/safe_conversions.h>

#include "dtls_transport.h"

SrtpTransport::SrtpTransport() {}

static const char kDtlsSrtpExporterLabel[] = "EXTRACTOR-dtls_srtp";
static const size_t kMaxSrtpHmacOverhead = 16;

void SrtpTransport::SetDtlsTransport(DtlsTransport* dtls_transport) {
  dtls_transport_ = dtls_transport;
  dtls_transport_->dtls_state_callback_list_.AddReceiver(
      this, [this](DtlsTransport* transport, const DtlsTransportState state) {
        OnDtlsState(transport, state);
      });
  dtls_transport_->SignalReadPacket.connect(this, &SrtpTransport::OnReadPacket);
}

void SrtpTransport::SetupRtpDtlsSrtp() {
  std::vector<int> send_extension_ids;
  std::vector<int> recv_extension_ids;
  if (send_extension_ids_) {
    send_extension_ids = *send_extension_ids_;
  }
  if (recv_extension_ids_) {
    recv_extension_ids = *recv_extension_ids_;
  }

  int selected_crypto_suite;
  rtc::ZeroOnFreeBuffer<unsigned char> send_key;
  rtc::ZeroOnFreeBuffer<unsigned char> recv_key;

  if (!ExtractParams(dtls_transport_, &selected_crypto_suite, &send_key,
                     &recv_key) ||
      !SetRtpParams(selected_crypto_suite, &send_key[0],
                    static_cast<int>(send_key.size()), send_extension_ids,
                    selected_crypto_suite, &recv_key[0],
                    static_cast<int>(recv_key.size()), recv_extension_ids)) {
    RTC_LOG(LS_WARNING) << "DTLS-SRTP key installation for RTP failed";
  }
}

bool SrtpTransport::ExtractParams(
    DtlsTransport* dtls_transport,
    int* selected_crypto_suite,
    rtc::ZeroOnFreeBuffer<unsigned char>* send_key,
    rtc::ZeroOnFreeBuffer<unsigned char>* recv_key) {
  // if (!dtls_transport || !dtls_transport->IsDtlsActive()) {
  //   return false;
  // }

  if (!dtls_transport->GetSrtpCryptoSuite(selected_crypto_suite)) {
    RTC_LOG(LS_ERROR) << "No DTLS-SRTP selected crypto suite";
    return false;
  }

  RTC_LOG(LS_INFO) << "Extracting keys from transport: ";

  int key_len;
  int salt_len;
  if (!rtc::GetSrtpKeyAndSaltLengths((*selected_crypto_suite), &key_len,
                                     &salt_len)) {
    RTC_LOG(LS_ERROR) << "Unknown DTLS-SRTP crypto suite"
                      << selected_crypto_suite;
    return false;
  }

  // OK, we're now doing DTLS (RFC 5764)
  rtc::ZeroOnFreeBuffer<unsigned char> dtls_buffer(key_len * 2 + salt_len * 2);

  // RFC 5705 exporter using the RFC 5764 parameters
  if (!dtls_transport->ExportKeyingMaterial(kDtlsSrtpExporterLabel, NULL, 0,
                                            false, &dtls_buffer[0],
                                            dtls_buffer.size())) {
    RTC_LOG(LS_WARNING) << "DTLS-SRTP key export failed";
    RTC_NOTREACHED();  // This should never happen
    return false;
  }

  // Sync up the keys with the DTLS-SRTP interface
  rtc::ZeroOnFreeBuffer<unsigned char> client_write_key(key_len + salt_len);
  rtc::ZeroOnFreeBuffer<unsigned char> server_write_key(key_len + salt_len);
  size_t offset = 0;
  memcpy(&client_write_key[0], &dtls_buffer[offset], key_len);
  offset += key_len;
  memcpy(&server_write_key[0], &dtls_buffer[offset], key_len);
  offset += key_len;
  memcpy(&client_write_key[key_len], &dtls_buffer[offset], salt_len);
  offset += salt_len;
  memcpy(&server_write_key[key_len], &dtls_buffer[offset], salt_len);

  rtc::SSLRole role;
  if (!dtls_transport->GetDtlsRole(&role)) {
    RTC_LOG(LS_WARNING) << "Failed to get the DTLS role.";
    return false;
  }

  if (role == rtc::SSL_SERVER) {
    *send_key = std::move(server_write_key);
    *recv_key = std::move(client_write_key);
  } else {
    *send_key = std::move(client_write_key);
    *recv_key = std::move(server_write_key);
  }
  return true;
}

void SrtpTransport::CreateSrtpSessions() {
  send_session_.reset(new cricket::SrtpSession());
  recv_session_.reset(new cricket::SrtpSession());
  // if (external_auth_enabled_) {
  //   send_session_->EnableExternalAuth();
  // }
}

bool SrtpTransport::SetRtpParams(int send_cs,
                                 const uint8_t* send_key,
                                 int send_key_len,
                                 const std::vector<int>& send_extension_ids,
                                 int recv_cs,
                                 const uint8_t* recv_key,
                                 int recv_key_len,
                                 const std::vector<int>& recv_extension_ids) {
  // If parameters are being set for the first time, we should create new SRTP
  // sessions and call "SetSend/SetRecv". Otherwise we should call
  // "UpdateSend"/"UpdateRecv" on the existing sessions, which will internally
  // call "srtp_update".
  bool new_sessions = false;
  if (!send_session_) {
    RTC_DCHECK(!recv_session_);
    CreateSrtpSessions();
    new_sessions = true;
  }
  bool ret = new_sessions
                 ? send_session_->SetSend(send_cs, send_key, send_key_len,
                                          send_extension_ids)
                 : send_session_->UpdateSend(send_cs, send_key, send_key_len,
                                             send_extension_ids);
  if (!ret) {
    ResetParams();
    return false;
  }

  ret = new_sessions ? recv_session_->SetRecv(recv_cs, recv_key, recv_key_len,
                                              recv_extension_ids)
                     : recv_session_->UpdateRecv(
                           recv_cs, recv_key, recv_key_len, recv_extension_ids);
  if (!ret) {
    ResetParams();
    return false;
  }

  RTC_LOG(LS_INFO) << "SRTP " << (new_sessions ? "activated" : "updated")
                   << " with negotiated parameters: send cipher_suite "
                   << send_cs << " recv cipher_suite " << recv_cs;
  // MaybeUpdateWritableState();
  return true;
}

void SrtpTransport::ResetParams() {
  send_session_ = nullptr;
  recv_session_ = nullptr;
  // send_rtcp_session_ = nullptr;
  // recv_rtcp_session_ = nullptr;
  // MaybeUpdateWritableState();
  RTC_LOG(LS_INFO) << "The params in SRTP transport are reset.";
}

void SrtpTransport::OnDtlsState(DtlsTransport* transport,
                                const DtlsTransportState state) {
  if (state != DtlsTransportState::DTLS_TRANSPORT_CONNECTED) {
    return;
  }
  RTC_LOG(INFO) << "Connected";

  SetupRtpDtlsSrtp();
}

void SrtpTransport::OnReadPacket(const char* data,
                                 size_t len,
                                 const int64_t packet_time_us,
                                 int flags) {
  // TRACE_EVENT0("webrtc", "RtpTransport::OnReadPacket");

  // When using RTCP multiplexing we might get RTCP packets on the RTP
  // transport. We check the RTP payload type to determine if it is RTCP.
  auto array_view = rtc::MakeArrayView(data, len);
  cricket::RtpPacketType packet_type = cricket::InferRtpPacketType(array_view);
  // Filter out the packet that is neither RTP nor RTCP.
  if (packet_type == cricket::RtpPacketType::kUnknown) {
    return;
  }

  // Protect ourselves against crazy data.
  if (!cricket::IsValidRtpPacketSize(packet_type, len)) {
    RTC_LOG(LS_ERROR) << "Dropping incoming "
                      << cricket::RtpPacketTypeToString(packet_type)
                      << " packet: wrong size=" << len;
    return;
  }

  rtc::CopyOnWriteBuffer packet(data, len);

  OnRtpPacketReceived(std::move(packet), packet_time_us);
  // if (packet_type == cricket::RtpPacketType::kRtcp) {
  //   OnRtcpPacketReceived(std::move(packet), packet_time_us);
  // } else {
  //   OnRtpPacketReceived(std::move(packet), packet_time_us);
  // }
}

void SrtpTransport::OnRtpPacketReceived(rtc::CopyOnWriteBuffer packet,
                                        int64_t packet_time_us) {
  // if (!IsSrtpActive()) {
  //   RTC_LOG(LS_WARNING)
  //       << "Inactive SRTP transport received an RTP packet. Drop it.";
  //   return;
  // }
  // TRACE_EVENT0("webrtc", "SRTP Decode");
  char* data = packet.MutableData<char>();
  int len = rtc::checked_cast<int>(packet.size());
  if (!UnprotectRtp(data, len, &len)) {
    int seq_num = -1;
    uint32_t ssrc = 0;
    cricket::GetRtpSeqNum(data, len, &seq_num);
    cricket::GetRtpSsrc(data, len, &ssrc);

    // Limit the error logging to avoid excessive logs when there are lots of
    // bad packets.
    const int kFailureLogThrottleCount = 100;
    if (decryption_failure_count_ % kFailureLogThrottleCount == 0) {
      RTC_LOG(LS_ERROR) << "Failed to unprotect RTP packet: size=" << len
                        << ", seqnum=" << seq_num << ", SSRC=" << ssrc
                        << ", previous failure count: "
                        << decryption_failure_count_;
    }
    ++decryption_failure_count_;
    return;
  }
  packet.SetSize(len);
  // DemuxPacket(std::move(packet), packet_time_us);

  packet_callback_list_.Send(std::move(packet));
  RTC_LOG(INFO) << "!!!!";
}

bool SrtpTransport::SendRtpPacket(const char* data2,
                                  size_t size,
                                  const rtc::PacketOptions& options) {
  auto packet = rtc::CopyOnWriteBuffer(data2, size);
  
  packet.EnsureCapacity(size+kMaxSrtpHmacOverhead);
  // if (!IsSrtpActive()) {
  //   RTC_LOG(LS_ERROR)
  //       << "Failed to send the packet because SRTP transport is inactive.";
  //   return false;
  // }
  rtc::PacketOptions updated_options = options;
  // TRACE_EVENT0("webrtc", "SRTP Encode");
  bool res;
  uint8_t* data = packet.MutableData();
  int len = rtc::checked_cast<int>(packet.size());
  // If ENABLE_EXTERNAL_AUTH flag is on then packet authentication is not done
  // inside libsrtp for a RTP packet. A external HMAC module will be writing
  // a fake HMAC value. This is ONLY done for a RTP packet.
  // Socket layer will update rtp sendtime extension header if present in
  // packet with current time before updating the HMAC.
  res = ProtectRtp(data, len, static_cast<int>(packet.capacity()), &len);

  if (!res) {
    int seq_num = -1;
    uint32_t ssrc = 0;
    cricket::GetRtpSeqNum(data, len, &seq_num);
    cricket::GetRtpSsrc(data, len, &ssrc);
    RTC_LOG(LS_ERROR) << "Failed to protect RTP packet: size=" << len
                      << ", seqnum=" << seq_num << ", SSRC=" << ssrc;
    return false;
  }

  // Update the length of the packet now that we've added the auth tag.
  packet.SetSize(len);

  dtls_transport_->ice_transport_->udp_transport_->SendPacket(packet.data(),
                                                              packet.size());
  return true;
}

bool SrtpTransport::ProtectRtp(void* p, int in_len, int max_len, int* out_len) {
  // if (!IsSrtpActive()) {
  //   RTC_LOG(LS_WARNING) << "Failed to ProtectRtp: SRTP not active";
  //   return false;
  // }
  RTC_CHECK(send_session_);
  return send_session_->ProtectRtp(p, in_len, max_len, out_len);
}

bool SrtpTransport::UnprotectRtp(void* p, int in_len, int* out_len) {
  // if (!IsSrtpActive()) {
  //   RTC_LOG(LS_WARNING) << "Failed to UnprotectRtp: SRTP not active";
  //   return false;
  // }
  RTC_CHECK(recv_session_);
  return recv_session_->UnprotectRtp(p, in_len, out_len);
}
