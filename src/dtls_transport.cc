#include "dtls_transport.h"

#include <p2p/base/dtls_transport_internal.h>
#include <rtc_base/ssl_fingerprint.h>
#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate.h"
#include "rtc_base/ssl_identity.h"

static const size_t kDtlsRecordHeaderLen = 13;
static const size_t kMaxDtlsPacketLen = 2048;
static const size_t kMinRtpPacketLen = 12;

// Maximum number of pending packets in the queue. Packets are read immediately
// after they have been written, so a capacity of "1" is sufficient.
//
// However, this bug seems to indicate that's not the case: crbug.com/1063834
// So, temporarily increasing it to 2 to see if that makes a difference.
static const size_t kMaxPendingPackets = 2;

// Minimum and maximum values for the initial DTLS handshake timeout. We'll pick
// an initial timeout based on ICE RTT estimates, but clamp it to this range.
static const int kMinHandshakeTimeout = 50;
static const int kMaxHandshakeTimeout = 3000;

static bool IsDtlsPacket(const char* data, size_t len) {
  const uint8_t* u = reinterpret_cast<const uint8_t*>(data);
  return (len >= kDtlsRecordHeaderLen && (u[0] > 19 && u[0] < 64));
}

static bool IsDtlsClientHelloPacket(const char* data, size_t len) {
  if (!IsDtlsPacket(data, len)) {
    return false;
  }
  const uint8_t* u = reinterpret_cast<const uint8_t*>(data);
  return len > 17 && u[0] == 22 && u[13] == 1;
}

static bool IsRtpPacket(const char* data, size_t len) {
  const uint8_t* u = reinterpret_cast<const uint8_t*>(data);
  return (len >= kMinRtpPacketLen && (u[0] & 0xC0) == 0x80);
}

StreamInterfaceChannel::StreamInterfaceChannel(IceTransport* ice_transport)
    : ice_transport_(ice_transport),
      state_(rtc::SS_OPEN),
      packets_(kMaxPendingPackets, kMaxDtlsPacketLen) {}

rtc::StreamResult StreamInterfaceChannel::Read(void* buffer,
                                               size_t buffer_len,
                                               size_t* read,
                                               int* error) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);

  if (state_ == rtc::SS_CLOSED) {
    return rtc::SR_EOS;
  }
  if (state_ == rtc::SS_OPENING) {
    return rtc::SR_BLOCK;
  }

  if (!packets_.ReadFront(buffer, buffer_len, read)) {
    return rtc::SR_BLOCK;
  }

  return rtc::SR_SUCCESS;
}

rtc::StreamResult StreamInterfaceChannel::Write(const void* data,
                                                size_t data_len,
                                                size_t* written,
                                                int* error) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  // Always succeeds, since this is an unreliable transport anyway.
  // TODO(zhihuang): Should this block if ice_transport_'s temporarily
  // unwritable?
  rtc::PacketOptions packet_options;
  ice_transport_->SendPacket(static_cast<const char*>(data), data_len,
                             packet_options);
  if (written) {
    *written = data_len;
  }
  return rtc::SR_SUCCESS;
}

bool StreamInterfaceChannel::OnPacketReceived(const char* data, size_t size) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  if (packets_.size() > 0) {
    RTC_LOG(LS_WARNING) << "Packet already in queue.";
  }
  bool ret = packets_.WriteBack(data, size, NULL);
  if (!ret) {
    // Somehow we received another packet before the SSLStreamAdapter read the
    // previous one out of our temporary buffer. In this case, we'll log an
    // error and still signal the read event, hoping that it will read the
    // packet currently in packets_.
    RTC_LOG(LS_ERROR) << "Failed to write packet to queue.";
  }
  SignalEvent(this, rtc::SE_READ, 0);
  return ret;
}

rtc::StreamState StreamInterfaceChannel::GetState() const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  return state_;
}

void StreamInterfaceChannel::Close() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  packets_.Clear();
  state_ = rtc::SS_CLOSED;
}

DtlsTransport::DtlsTransport(IceTransport* ice_transport,
                             const webrtc::CryptoOptions& crypto_options,
                             rtc::SSLProtocolVersion max_version)
    : ice_transport_(ice_transport),
      downward_(NULL),
      srtp_ciphers_(crypto_options.GetSupportedDtlsSrtpCryptoSuites()),
      ssl_max_version_(max_version) {}

void DtlsTransport::Init(bool is_client) {
  ice_transport_->SignalReadPacket.connect(this, &DtlsTransport::OnPacket);
  // ice_transport_->Init();
  // FIXME(CC): set by webrtc transport?
  if (is_client) {
    this->SetDtlsRole(rtc::SSLRole::SSL_CLIENT);
  } else {
    this->SetDtlsRole(rtc::SSLRole::SSL_SERVER);
  }
  // this->SetDtlsRole(rtc::SSLRole::SSL_CLIENT);
}

auto DtlsTransport::certificate_ = rtc::RTCCertificate::Create(
    rtc::SSLIdentity::Create("name_", rtc::KT_DEFAULT));

const rtc::scoped_refptr<rtc::RTCCertificate>& DtlsTransport::certificate() {
  return certificate_;
}

DtlsTransportState DtlsTransport::dtls_state() const {
  return dtls_state_;
}

bool DtlsTransport::SetDtlsRole(rtc::SSLRole role) {
  if (dtls_) {
    RTC_DCHECK(dtls_role_);
    if (*dtls_role_ != role) {
      RTC_LOG(LS_ERROR)
          << "SSL Role can't be reversed after the session is setup.";
      return false;
    }
    return true;
  }

  dtls_role_ = role;
  return true;
}

bool DtlsTransport::GetDtlsRole(rtc::SSLRole* role) const {
  if (!dtls_role_) {
    return false;
  }
  *role = *dtls_role_;
  return true;
}

void DtlsTransport::MaybeStartDtls() {
  if (dtls_ && ice_transport_->writable()) {
    ConfigureHandshakeTimeout();

    if (dtls_->StartSSL()) {
      // This should never fail:
      // Because we are operating in a nonblocking mode and all
      // incoming packets come in via OnReadPacket(), which rejects
      // packets in this state, the incoming queue must be empty. We
      // ignore write errors, thus any errors must be because of
      // configuration and therefore are our fault.
      RTC_NOTREACHED() << "StartSSL failed.";
      RTC_LOG(LS_ERROR) << ToString() << ": Couldn't start DTLS handshake";
      set_dtls_state(DTLS_TRANSPORT_FAILED);
      return;
    }
    RTC_LOG(LS_INFO) << ToString() << ": DtlsTransport: Started DTLS handshake";
    set_dtls_state(DTLS_TRANSPORT_CONNECTING);
    // Now that the handshake has started, we can process a cached ClientHello
    // (if one exists).
    if (cached_client_hello_.size()) {
      if (*dtls_role_ == rtc::SSL_SERVER) {
        RTC_LOG(LS_INFO) << ToString()
                         << ": Handling cached DTLS ClientHello packet.";
        if (!HandleDtlsPacket(cached_client_hello_.data<char>(),
                              cached_client_hello_.size())) {
          RTC_LOG(LS_ERROR) << ToString() << ": Failed to handle DTLS packet.";
        }
      } else {
        RTC_LOG(LS_WARNING) << ToString()
                            << ": Discarding cached DTLS ClientHello packet "
                               "because we don't have the server role.";
      }
      cached_client_hello_.Clear();
    }
  }
}

void DtlsTransport::ConfigureHandshakeTimeout() {
  RTC_DCHECK(dtls_);
  // TODO(CC): maybe implement
  // absl::optional<int> rtt = ice_transport_->GetRttEstimate();
  // if (rtt) {
  //   // Limit the timeout to a reasonable range in case the ICE RTT takes
  //   // extreme values.
  //   int initial_timeout = std::max(kMinHandshakeTimeout,
  //                                  std::min(kMaxHandshakeTimeout, 2 *
  //                                  (*rtt)));
  //   RTC_LOG(LS_INFO) << ToString() << ": configuring DTLS handshake timeout "
  //                    << initial_timeout << " based on ICE RTT " << *rtt;

  //   dtls_->SetInitialRetransmissionTimeout(initial_timeout);
  // } else {
  //   RTC_LOG(LS_INFO)
  //       << ToString()
  //       << ": no RTT estimate - using default DTLS handshake timeout";
  // }
}

bool DtlsTransport::SetupDtls() {
  RTC_DCHECK(dtls_role_);
  {
    auto downward = std::make_unique<StreamInterfaceChannel>(ice_transport_);
    StreamInterfaceChannel* downward_ptr = downward.get();

    dtls_ = rtc::SSLStreamAdapter::Create(std::move(downward));
    if (!dtls_) {
      RTC_LOG(LS_ERROR) << ToString() << ": Failed to create DTLS adapter.";
      return false;
    }
    downward_ = downward_ptr;
  }

  dtls_->SetIdentity(local_certificate_->identity()->Clone());
  dtls_->SetMode(rtc::SSL_MODE_DTLS);
  dtls_->SetMaxProtocolVersion(ssl_max_version_);
  dtls_->SetServerRole(*dtls_role_);
  dtls_->SignalEvent.connect(this, &DtlsTransport::OnDtlsEvent);
  dtls_->SignalSSLHandshakeError.connect(this,
                                         &DtlsTransport::OnDtlsHandshakeError);

  rtc::SSLPeerCertificateDigestError err;
  if (remote_fingerprint_value_.size() &&
      !dtls_->SetPeerCertificateDigest(
          remote_fingerprint_algorithm_,
          reinterpret_cast<unsigned char*>(remote_fingerprint_value_.data()),
          remote_fingerprint_value_.size(), &err)) {
    RTC_LOG(LS_ERROR) << ": Couldn't set DTLS certificate digest.";
    return false;
  }

  // Set up DTLS-SRTP, if it's been enabled.
  if (!srtp_ciphers_.empty()) {
    if (!dtls_->SetDtlsSrtpCryptoSuites(srtp_ciphers_)) {
      RTC_LOG(LS_ERROR) << ToString() << ": Couldn't set DTLS-SRTP ciphers.";
      return false;
    }
  } else {
    RTC_LOG(LS_INFO) << ToString() << ": Not using DTLS-SRTP.";
  }

  RTC_LOG(LS_INFO) << ToString() << ": DTLS setup complete.";

  // If the underlying ice_transport is already writable at this point, we may
  // be able to start DTLS right away.
  MaybeStartDtls();
  return true;
}

void DtlsTransport::OnDtlsHandshakeError(rtc::SSLHandshakeError error) {
  // SendDtlsHandshakeError(error);
}

bool DtlsTransport::SetLocalCertificate(
    const rtc::scoped_refptr<rtc::RTCCertificate>& certificate) {
  if (dtls_active_) {
    if (certificate == local_certificate_) {
      // This may happen during renegotiation.
      RTC_LOG(LS_INFO) << ToString() << ": Ignoring identical DTLS identity";
      return true;
    } else {
      RTC_LOG(LS_ERROR) << ToString()
                        << ": Can't change DTLS local identity in this state";
      return false;
    }
  }

  if (certificate) {
    local_certificate_ = certificate;
    dtls_active_ = true;
  } else {
    RTC_LOG(LS_INFO) << ToString()
                     << ": NULL DTLS identity supplied. Not doing DTLS";
  }

  return true;
}

rtc::scoped_refptr<rtc::RTCCertificate> DtlsTransport::GetLocalCertificate()
    const {
  return local_certificate_;
}

bool DtlsTransport::SetRemoteFingerprint(const std::string& algorithm,
                                         const std::string& fingerprint) {
  auto f = rtc::SSLFingerprint::CreateFromRfc4572(algorithm, fingerprint);
  return SetRemoteFingerprint(
      algorithm, reinterpret_cast<const uint8_t*>(f->digest.data()),
      f->digest.size());
}

bool DtlsTransport::SetRemoteFingerprint(const std::string& digest_alg,
                                         const uint8_t* digest,
                                         size_t digest_len) {
  RTC_LOG(INFO) << "Dtls SetRemoteFingerprint";

  rtc::Buffer remote_fingerprint_value(digest, digest_len);

  // Once we have the local certificate, the same remote fingerprint can be set
  // multiple times.
  if (dtls_active_ && remote_fingerprint_value_ == remote_fingerprint_value &&
      !digest_alg.empty()) {
    // This may happen during renegotiation.
    RTC_LOG(LS_INFO) << ToString()
                     << ": Ignoring identical remote DTLS fingerprint";
    return true;
  }

  // If the other side doesn't support DTLS, turn off |dtls_active_|.
  // TODO(deadbeef): Remove this. It's dangerous, because it relies on higher
  // level code to ensure DTLS is actually used, but there are tests that
  // depend on it, for the case where an m= section is rejected. In that case
  // SetRemoteFingerprint shouldn't even be called though.
  if (digest_alg.empty()) {
    RTC_DCHECK(!digest_len);
    RTC_LOG(LS_INFO) << ToString() << ": Other side didn't support DTLS.";
    dtls_active_ = false;
    return true;
  }

  // Otherwise, we must have a local certificate before setting remote
  // fingerprint.
  if (!dtls_active_) {
    RTC_LOG(LS_ERROR) << ToString()
                      << ": Can't set DTLS remote settings in this state.";
    return false;
  }

  // At this point we know we are doing DTLS
  bool fingerprint_changing = remote_fingerprint_value_.size() > 0u;
  remote_fingerprint_value_ = std::move(remote_fingerprint_value);
  remote_fingerprint_algorithm_ = digest_alg;

  if (dtls_ && !fingerprint_changing) {
    // This can occur if DTLS is set up before a remote fingerprint is
    // received. For instance, if we set up DTLS due to receiving an early
    // ClientHello.
    rtc::SSLPeerCertificateDigestError err;
    if (!dtls_->SetPeerCertificateDigest(
            remote_fingerprint_algorithm_,
            reinterpret_cast<unsigned char*>(remote_fingerprint_value_.data()),
            remote_fingerprint_value_.size(), &err)) {
      RTC_LOG(LS_ERROR) << ToString()
                        << ": Couldn't set DTLS certificate digest.";
      set_dtls_state(DTLS_TRANSPORT_FAILED);
      // If the error is "verification failed", don't return false, because
      // this means the fingerprint was formatted correctly but didn't match
      // the certificate from the DTLS handshake. Thus the DTLS state should go
      // to "failed", but SetRemoteDescription shouldn't fail.
      return err == rtc::SSLPeerCertificateDigestError::VERIFICATION_FAILED;
    }
    return true;
  }

  // If the fingerprint is changing, we'll tear down the DTLS association and
  // create a new one, resetting our state.
  if (dtls_ && fingerprint_changing) {
    dtls_.reset(nullptr);
    set_dtls_state(DTLS_TRANSPORT_NEW);
    // set_writable(false);
  }

  if (!SetupDtls()) {
    set_dtls_state(DTLS_TRANSPORT_FAILED);
    return false;
  }

  return true;
}

void DtlsTransport::OnPacket(const char* data,
                             size_t size,
                             const int64_t packet_time_us) {
  // RTC_LOG(INFO) << "OnPacket";
  switch (dtls_state()) {
    case DTLS_TRANSPORT_NEW:
      if (dtls_) {
        RTC_LOG(LS_INFO) << ToString()
                         << ": Packet received before DTLS started.";
      } else {
        RTC_LOG(LS_WARNING) << ToString()
                            << ": Packet received before we know if we are "
                               "doing DTLS or not.";
      }
      // Cache a client hello packet received before DTLS has actually started.
      if (IsDtlsClientHelloPacket(data, size)) {
        RTC_LOG(LS_INFO) << ToString()
                         << ": Caching DTLS ClientHello packet until DTLS is "
                            "started.";
        cached_client_hello_.SetData(data, size);
        // If we haven't started setting up DTLS yet (because we don't have a
        // remote fingerprint/role), we can use the client hello as a clue that
        // the peer has chosen the client role, and proceed with the handshake.
        // The fingerprint will be verified when it's set.
        if (!dtls_ && local_certificate_) {
          SetDtlsRole(rtc::SSL_SERVER);
          SetupDtls();
        }
      } else {
        RTC_LOG(LS_INFO) << ToString()
                         << ": Not a DTLS ClientHello packet; dropping.";
      }
      break;

    case DTLS_TRANSPORT_CONNECTING:
    case DTLS_TRANSPORT_CONNECTED:
      // We should only get DTLS or SRTP packets; STUN's already been demuxed.
      // Is this potentially a DTLS packet?
      if (IsDtlsPacket(data, size)) {
        if (!HandleDtlsPacket(data, size)) {
          RTC_LOG(LS_ERROR) << ToString() << ": Failed to handle DTLS packet.";
          return;
        }
      } else {
        // Not a DTLS packet; our handshake should be complete by now.
        if (dtls_state() != DTLS_TRANSPORT_CONNECTED) {
          RTC_LOG(LS_ERROR) << ToString()
                            << ": Received non-DTLS packet before DTLS "
                               "complete.";
          return;
        }

        // And it had better be a SRTP packet.
        if (!IsRtpPacket(data, size)) {
          RTC_LOG(LS_ERROR)
              << ToString() << ": Received unexpected non-DTLS packet.";
          return;
        }

        // Sanity check.
        RTC_DCHECK(!srtp_ciphers_.empty());

        // Signal this upwards as a bypass packet.
        // TODO(CC):
        SignalReadPacket(data, size, packet_time_us, cricket::PF_SRTP_BYPASS);
      }
      break;
    case DTLS_TRANSPORT_FAILED:
    case DTLS_TRANSPORT_CLOSED:
      // This shouldn't be happening. Drop the packet.
      break;
  }
}

bool DtlsTransport::HandleDtlsPacket(const char* data, size_t size) {
  // Sanity check we're not passing junk that
  // just looks like DTLS.
  const uint8_t* tmp_data = reinterpret_cast<const uint8_t*>(data);
  size_t tmp_size = size;
  while (tmp_size > 0) {
    if (tmp_size < kDtlsRecordHeaderLen) {
      return false;  // Too short for the header
    }

    size_t record_len = (tmp_data[11] << 8) | (tmp_data[12]);
    if ((record_len + kDtlsRecordHeaderLen) > tmp_size) {
      return false;  // Body too short
    }

    tmp_data += record_len + kDtlsRecordHeaderLen;
    tmp_size -= record_len + kDtlsRecordHeaderLen;
  }

  // Looks good. Pass to the SIC which ends up being passed to
  // the DTLS stack.
  return downward_->OnPacketReceived(data, size);
}

void DtlsTransport::SendDtlsState(DtlsTransport* transport,
                                  DtlsTransportState state) {
  dtls_state_callback_list_.Send(transport, state);
}

bool DtlsTransport::GetSrtpCryptoSuite(int* cipher) {
  if (dtls_state() != DTLS_TRANSPORT_CONNECTED) {
    return false;
  }

  return dtls_->GetDtlsSrtpCryptoSuite(cipher);
}

bool DtlsTransport::ExportKeyingMaterial(const std::string& label,
                                         const uint8_t* context,
                                         size_t context_len,
                                         bool use_context,
                                         uint8_t* result,
                                         size_t result_len) {
  return (dtls_.get())
             ? dtls_->ExportKeyingMaterial(label, context, context_len,
                                           use_context, result, result_len)
             : false;
}

void DtlsTransport::set_dtls_state(DtlsTransportState state) {
  if (dtls_state_ == state) {
    return;
  }
  // if (event_log_) {
  //   event_log_->Log(std::make_unique<webrtc::RtcEventDtlsTransportState>(
  //       ConvertDtlsTransportState(state)));
  // }
  RTC_LOG(LS_VERBOSE) << ToString() << ": set_dtls_state from:" << dtls_state_
                      << " to " << state;
  dtls_state_ = state;
  SendDtlsState(this, state);
}

void DtlsTransport::OnDtlsEvent(rtc::StreamInterface* dtls, int sig, int err) {
  RTC_DCHECK_RUN_ON(&thread_checker_);
  RTC_DCHECK(dtls == dtls_.get());
  if (sig & rtc::SE_OPEN) {
    // This is the first time.
    RTC_LOG(LS_INFO) << ToString() << ": DTLS handshake complete.";
    if (dtls_->GetState() == rtc::SS_OPEN) {
      // The check for OPEN shouldn't be necessary but let's make
      // sure we don't accidentally frob the state if it's closed.
      set_dtls_state(DTLS_TRANSPORT_CONNECTED);
      // set_writable(true);
    }
  }
  if (sig & rtc::SE_READ) {
    char buf[kMaxDtlsPacketLen];
    size_t read;
    int read_error;
    rtc::StreamResult ret;
    // The underlying DTLS stream may have received multiple DTLS records in
    // one packet, so read all of them.
    do {
      ret = dtls_->Read(buf, sizeof(buf), &read, &read_error);
      if (ret == rtc::SR_SUCCESS) {
        // SignalReadPacket(this, buf, read, rtc::TimeMicros(), 0);
      } else if (ret == rtc::SR_EOS) {
        // Remote peer shut down the association with no error.
        RTC_LOG(LS_INFO) << ToString() << ": DTLS transport closed by remote";
        // set_writable(false);
        set_dtls_state(DTLS_TRANSPORT_CLOSED);
        // SignalClosed(this);
      } else if (ret == rtc::SR_ERROR) {
        // Remote peer shut down the association with an error.
        RTC_LOG(LS_INFO)
            << ToString()
            << ": Closed by remote with DTLS transport error, code="
            << read_error;
        // set_writable(false);
        set_dtls_state(DTLS_TRANSPORT_FAILED);
        // SignalClosed(this);
      }
    } while (ret == rtc::SR_SUCCESS);
  }
  if (sig & rtc::SE_CLOSE) {
    RTC_DCHECK(sig == rtc::SE_CLOSE);  // SE_CLOSE should be by itself.
    // set_writable(false);
    if (!err) {
      RTC_LOG(LS_INFO) << ToString() << ": DTLS transport closed";
      set_dtls_state(DTLS_TRANSPORT_CLOSED);
    } else {
      RTC_LOG(LS_INFO) << ToString() << ": DTLS transport error, code=" << err;
      set_dtls_state(DTLS_TRANSPORT_FAILED);
    }
  }
}
