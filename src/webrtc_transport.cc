
#include "webrtc_transport.h"

#include <iostream>
#include <type_traits>

#include <api/transport/stun.h>
#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "tools.h"

using namespace cricket;

WebrtcTransport::WebrtcTransport(const std::string& direction,
                                 const std::string& ip,
                                 const int port) {
  direction_ = direction;
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
    srtp->packet_callback_list_.AddReceiver(
        this, [this](rtc::CopyOnWriteBuffer packet) {
          this->packet_callback_list_.Send(std::move(packet));
        });
    srtp->SetDtlsTransport(dtls_transport_);
    return srtp;
  });
}

void WebrtcTransport::Init() {
  thread_->PostTask(webrtc::ToQueuedTask([this]() {
    bool is_client = true;
    if (direction_ == "recvonly") {
      is_client = false;
    }
    packet_callback_list_.AddReceiver(
        [](rtc::CopyOnWriteBuffer packet) { RTC_LOG(INFO) << "packet"; });

    dtls_transport_->Init(is_client);
    ice_transport_->Init();
  }));
}

void WebrtcTransport::OnPacket(const char* data,
                               size_t size,
                               const int64_t timestamp) {
  auto packet = rtc::CopyOnWriteBuffer(data, size);

  thread_->PostTask(
      webrtc::ToQueuedTask([this, packet_copy = std::move(packet)]() {
        rtc::PacketOptions packet_options;

        srtp_transport_->SendRtpPacket(
            reinterpret_cast<const char*>(packet_copy.data()),
            packet_copy.size(), packet_options);
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

Sender* WebrtcTransport::CreateSender(json codec) {
  auto* sender = new Sender();

  return sender;
}