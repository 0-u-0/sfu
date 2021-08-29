#ifndef SRC_UDP_TRANSPORT
#define SRC_UDP_TRANSPORT

#include <functional>

#include <rtc_base/async_udp_socket.h>
#include <rtc_base/third_party/sigslot/sigslot.h>
#include <rtc_base/thread.h>

#include "logger.h"

using UdpDataCallback = std::function<
    void(const char* data, size_t len, const rtc::SocketAddress& addr)>;

class UdpTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  UdpTransport(const std::string& ip, const int port);

  void Init();
  void SetRemoteAddress(const std::string& ip, int port);

  void SendPacket(const uint8_t* data, size_t size);
  void SendTo(const uint8_t* data, size_t size, const rtc::SocketAddress& addr);
  void OnUdpPacket(rtc::AsyncPacketSocket* socket,
                   const char* data,
                   size_t size,
                   const rtc::SocketAddress& addr,
                   const int64_t& timestamp);

  UdpDataCallback data_callback_ = nullptr;
  std::unique_ptr<rtc::Thread> thread_;
  rtc::AsyncUDPSocket* rtp_socket_;
  rtc::SocketAddress local_address_;
  rtc::SocketAddress remote_address_;
};

#endif /* SRC_UDP_TRANSPORT */
