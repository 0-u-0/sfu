
#ifndef SRC_CORE_NEW_UDP_TRANSPORT_H_
#define SRC_CORE_NEW_UDP_TRANSPORT_H_

#include <rtc_base/async_udp_socket.h>
#include <rtc_base/callback_list.h>
#include <rtc_base/thread.h>

#include "common/logger.h"

namespace webrtc {

class NewUdpTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  NewUdpTransport(const std::string& ip, int port);
  ~NewUdpTransport();

  bool Init();

  void SetRemoteAddress(const std::string& ip, const int port);
  void SetRemoteAddress(const rtc::SocketAddress& remote_address);

  void Send(const uint8_t* data, size_t size);
  void SendTo(const uint8_t* data, size_t size, const rtc::SocketAddress& addr);

  void OnPacket(rtc::AsyncPacketSocket* socket,
                const char* data,
                size_t size,
                const rtc::SocketAddress& addr,
                const int64_t& timestamp);
  void Close();

  int GetLocalPort();

  webrtc::CallbackList<const char*,
                       size_t,
                       const rtc::SocketAddress&,
                       const int64_t&>
      data_callback_list_;
  rtc::AsyncUDPSocket* socket_ = nullptr;

  rtc::SocketAddress local_address_;
  rtc::SocketAddress remote_address_;
  // std::unique_ptr<rtc::Thread> thread_;
  rtc::Thread* thread_;

  std::string ip_;
  int port_;
};
}  // namespace webrtc

#endif /* SRC_CORE_NEW_UDP_TRANSPORT_H_ */
