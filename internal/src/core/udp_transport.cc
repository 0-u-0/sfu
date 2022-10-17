
#include "udp_transport.h"

#include <rtc_base/copy_on_write_buffer.h>
#include <rtc_base/task_utils/to_queued_task.h>

namespace webrtc {
DEFINE_LOGGER(NewUdpTransport, "NewUdpTransport");

NewUdpTransport::NewUdpTransport(const std::string& ip, int port) {
  ip_ = ip;
  port_ = port;
  local_address_ = rtc::SocketAddress(ip, port);

  thread_ = rtc::Thread::Current();
}

NewUdpTransport::~NewUdpTransport() {
  Close();
}

int NewUdpTransport::GetLocalPort() {
  return port_;
}

bool NewUdpTransport::Init() {
  return thread_->Invoke<bool>(RTC_FROM_HERE, [this]() {
    socket_ =
        rtc::AsyncUDPSocket::Create(thread_->socketserver(), local_address_);
    if (socket_ == nullptr) {
      ELOG("Create Socket failed, ip: {}, port: {}", ip_, port_);
      return false;
    } else {
      socket_->SignalReadPacket.connect(this, &NewUdpTransport::OnPacket);
      return true;
    }
  });
}

void NewUdpTransport::SetRemoteAddress(const std::string& ip, int port) {
  thread_->Invoke<void>(RTC_FROM_HERE, [this, ip, port]() {
    remote_address_ = rtc::SocketAddress(ip, port);
  });
}

void NewUdpTransport::SetRemoteAddress(
    const rtc::SocketAddress& remote_address) {
  thread_->Invoke<void>(RTC_FROM_HERE, [this, remote_address]() {
    remote_address_ = remote_address;
  });
}

void NewUdpTransport::OnPacket(rtc::AsyncPacketSocket* socket,
                               const char* data,
                               size_t size,
                               const rtc::SocketAddress& addr,
                               const int64_t& timestamp) {
  data_callback_list_.Send(data, size, addr, timestamp);
}

void NewUdpTransport::Send(const uint8_t* data, size_t size) {
  SendTo(data, size, remote_address_);
}

void NewUdpTransport::SendTo(const uint8_t* data,
                             size_t size,
                             const rtc::SocketAddress& addr) {
  auto packet = rtc::CopyOnWriteBuffer(data, size);

  thread_->PostTask(
      ToQueuedTask([this, packet_copy = std::move(packet), addr]() {
        if (!addr.IsNil() && socket_ != nullptr) {
          if (!socket_->SendTo(packet_copy.data(), packet_copy.size(), addr,
                               rtc::PacketOptions())) {
            // LOG_WARN("Send failed");
          }
        }
      }));
}

void NewUdpTransport::Close() {
  // TODO(CC): handle
  thread_->Invoke<void>(RTC_FROM_HERE, [this]() {
    if (socket_ != nullptr) {
      // TODO(CC): maybe handle socket error
      socket_->Close();
      delete socket_;
      socket_ = nullptr;
    }
  });
}

}  // namespace webrtc
