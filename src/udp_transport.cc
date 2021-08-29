
#include "udp_transport.h"

#include <rtc_base/task_utils/to_queued_task.h>

DEFINE_LOGGER(UdpTransport, "UdpTransport");

UdpTransport::UdpTransport(const std::string& ip, const int port) {
  local_address_ = rtc::SocketAddress(ip, port);

  thread_ = rtc::Thread::CreateWithSocketServer();
}

void UdpTransport::Init() {
  thread_->Start();
  thread_->PostTask(webrtc::ToQueuedTask([this]() {
    rtp_socket_ =
        rtc::AsyncUDPSocket::Create(thread_->socketserver(), local_address_);

    rtp_socket_->SignalReadPacket.connect(this, &UdpTransport::OnUdpPacket);
  }));
}

void UdpTransport::SetRemoteAddress(const std::string& ip, int port) {
  remote_address_ = rtc::SocketAddress(ip, port);
}

void UdpTransport::OnUdpPacket(rtc::AsyncPacketSocket* socket,
                               const char* data,
                               size_t size,
                               const rtc::SocketAddress& addr,
                               const int64_t& timestamp) {
  ELOG_DEBUG("Get data from %s", addr.ToString().c_str());
  if (data_callback_ != nullptr) {
    data_callback_(data, size, addr);
  }
}

void UdpTransport::SendPacket(const uint8_t* data, size_t size) {
  if (remote_address_.port() != 0) {
    std::vector<uint8_t> packet_copy(data, data + size);

    thread_->PostTask(
        webrtc::ToQueuedTask([this, packet_copy2 = std::move(packet_copy)]() {
          if (!rtp_socket_->SendTo(packet_copy2.data(), packet_copy2.size(),
                                   remote_address_, rtc::PacketOptions())) {
            ELOG_WARN("Send failed");
          }
        }));
  }
}

void UdpTransport::SendTo(const uint8_t* data,
                          size_t size,
                          const rtc::SocketAddress& addr) {
  std::vector<uint8_t> packet_copy(data, data + size);

  thread_->PostTask(webrtc::ToQueuedTask(
      [this, addr, packet_copy2 = std::move(packet_copy)]() {
        if (!rtp_socket_->SendTo(packet_copy2.data(), packet_copy2.size(), addr,
                                 rtc::PacketOptions())) {
          ELOG_WARN("Send failed");
        }
      }));
}