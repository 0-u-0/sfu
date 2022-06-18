#ifndef SRC_UDP_TRANSPORT
#define SRC_UDP_TRANSPORT

#include "api/sequence_checker.h"
#include "rtc_base/async_udp_socket.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/thread.h"

#include "common/logger.h"

class UdpTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  UdpTransport(const std::string& ip, const int port);

  void Init() RTC_RUN_ON(thread_);
  void SetRemoteAddress(const std::string& ip, int port) RTC_RUN_ON(thread_);
  void SetRemoteAddress(const rtc::SocketAddress& remote_address)
      RTC_RUN_ON(thread_);

  void SendPacket(const uint8_t* data, size_t size) RTC_RUN_ON(thread_);
  void SendTo(const uint8_t* data, size_t size, const rtc::SocketAddress& addr)
      RTC_RUN_ON(thread_);
  void OnPacket(rtc::AsyncPacketSocket* socket,
                const char* data,
                size_t size,
                const rtc::SocketAddress& addr,
                const int64_t& timestamp);

  sigslot::
      signal4<const char*, size_t, const rtc::SocketAddress&, const int64_t>
          emit_packet_;

  rtc::Thread* thread_;
  rtc::AsyncUDPSocket* rtp_socket_;
  rtc::SocketAddress local_address_;
  rtc::SocketAddress remote_address_;
};

#endif /* SRC_UDP_TRANSPORT */
