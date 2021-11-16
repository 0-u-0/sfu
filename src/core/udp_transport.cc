
#include "core/udp_transport.h"

#include "rtc_base/task_utils/to_queued_task.h"

DEFINE_LOGGER(UdpTransport, "UdpTransport");

UdpTransport::UdpTransport(const std::string& ip, const int port) {
  local_address_ = rtc::SocketAddress(ip, port);
  thread_ = rtc::Thread::Current();
}

void UdpTransport::Init() {
  rtp_socket_ =
      rtc::AsyncUDPSocket::Create(thread_->socketserver(), local_address_);

  rtp_socket_->SignalReadPacket.connect(this, &UdpTransport::OnPacket);
}

void UdpTransport::SetRemoteAddress(const std::string& ip, int port) {
  remote_address_ = rtc::SocketAddress(ip, port);
}
void UdpTransport::SetRemoteAddress(const rtc::SocketAddress& remote_address) {
  remote_address_ = remote_address;
}

void UdpTransport::OnPacket(rtc::AsyncPacketSocket* socket,
                            const char* data,
                            size_t size,
                            const rtc::SocketAddress& addr,
                            const int64_t& timestamp) {
  DLOG("Get data from {}", addr.ToString());
  emit_packet_(data, size, addr, timestamp);
}

void UdpTransport::SendPacket(const uint8_t* data, size_t size) {
  if (!remote_address_.IsNil()) {
    if (!rtp_socket_->SendTo(data, size, remote_address_,
                             rtc::PacketOptions())) {
      // LOG_WARN("Send failed");
    }
  }
}

void UdpTransport::SendTo(const uint8_t* data,
                          size_t size,
                          const rtc::SocketAddress& addr) {
  if (!rtp_socket_->SendTo(data, size, addr, rtc::PacketOptions())) {
    // LOG_WARN("Send failed");
  }
}
