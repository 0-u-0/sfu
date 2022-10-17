
#include "core/rtp_transport.h"

#include <rtc_base/task_utils/to_queued_task.h>

#include "core/udp_transport.h"

RtpTransport::RtpTransport(const std::string& ip, const int port) {
  thread_ = rtc::Thread::CreateWithSocketServer();
  thread_->Start();

  udp_transport_ = thread_->Invoke<UdpTransport*>(
      RTC_FROM_HERE, [ip, port]() { return new UdpTransport(ip, port); });
}

void RtpTransport::Init() {
  thread_->PostTask(webrtc::ToQueuedTask([this]() {
    udp_transport_->Init();
    udp_transport_->on_packet_.connect(this, &RtpTransport::OnPacket);
  }));
}

void RtpTransport::OnPacket(const char* data,
                            size_t size,
                            const rtc::SocketAddress& addr,
                            const int64_t timestamp) {
  SignalReadPacket(data, size, timestamp);
}

void RtpTransport::SetRemoteAddress(const std::string& ip, const int port) {
  thread_->PostTask(webrtc::ToQueuedTask(
      [this, &ip, port]() { udp_transport_->SetRemoteAddress(ip, port); }));
}

void RtpTransport::SendPacket(rtc::CopyOnWriteBuffer packet) {
  thread_->PostTask(
      webrtc::ToQueuedTask([this, packet_copy = std::move(packet)]() {
        udp_transport_->SendPacket(packet_copy.data(), packet_copy.size());
      }));
}