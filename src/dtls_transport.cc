#include "dtls_transport.h"

#include "rtc_base/logging.h"
#include "rtc_base/rtc_certificate.h"
#include "rtc_base/ssl_identity.h"


static const size_t kDtlsRecordHeaderLen = 13;
static const size_t kMaxDtlsPacketLen = 2048;
static const size_t kMinRtpPacketLen = 12;

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

bool DtlsTransport::SetupDtls() {
  return true;
}

void DtlsTransport::OnPacket(const char* data, size_t size) {
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
        //TODO(CC): 
        // SignalReadPacket(this, data, size, packet_time_us, PF_SRTP_BYPASS);
      }
      break;
    case DTLS_TRANSPORT_FAILED:
    case DTLS_TRANSPORT_CLOSED:
      // This shouldn't be happening. Drop the packet.
      break;
  }
}

bool DtlsTransport::HandleDtlsPacket(const char* data, size_t size) {
  return true;
}
