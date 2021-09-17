
#include "webrtc_transport.h"

#include <iostream>

#include <api/transport/stun.h>
#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "tools.h"

using namespace cricket;

WebrtcTransport::WebrtcTransport(const std::string& ip, const int port) {
  thread_ = rtc::Thread::CreateWithSocketServer();
  thread_->Start();

  ice_transport_ = thread_->Invoke<IceTransport*>(
      RTC_FROM_HERE, [ip, port]() { return new IceTransport(ip, port); });
  dtls_transport_ = thread_->Invoke<DtlsTransport*>(RTC_FROM_HERE, [this]() {
    return new DtlsTransport(ice_transport_, webrtc::CryptoOptions(),
                             rtc::SSL_PROTOCOL_DTLS_12);
  });

  srtp_transport_ = thread_->Invoke<SrtpTransport*>(RTC_FROM_HERE, [this]() {
    auto srtp = new SrtpTransport();
    srtp->SetDtlsTransport(dtls_transport_);
    return srtp;
  });
}

void WebrtcTransport::Init() {
  thread_->PostTask(webrtc::ToQueuedTask([this]() {
    dtls_transport_->Init();
    ice_transport_->Init();
  }));
}

bool WebrtcTransport::SetLocalCertificate(
    const rtc::scoped_refptr<rtc::RTCCertificate>& certificate) {
  return thread_->Invoke<bool>(RTC_FROM_HERE, [this, certificate]() {
    return dtls_transport_->SetLocalCertificate(certificate);
  });
}

bool WebrtcTransport::SetRemoteFingerprint(const std::string& algorithm,
                                           const std::string& fingerprint) {
  return thread_->Invoke<bool>(RTC_FROM_HERE, [this, algorithm, fingerprint]() {
    return dtls_transport_->SetRemoteFingerprint(algorithm, fingerprint);
  });
}