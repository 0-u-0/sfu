#ifndef SRC_RTP_TRANSPORT_H_
#define SRC_RTP_TRANSPORT_H_

#include <rtc_base/copy_on_write_buffer.h>
#include <rtc_base/thread.h>

#include "udp_transport.h"

class RtpTransport : public sigslot::has_slots<> {
 public:
  RtpTransport(const std::string& ip, const int port);
  void Init();
  void SetRemoteAddress(const std::string& ip, const int port);
  void SendPacket(rtc::CopyOnWriteBuffer packet);

  void OnPacket(const char* data,
                size_t size,
                const rtc::SocketAddress& addr,
                const int64_t timestamp);

  sigslot::signal3<const char*, size_t, const int64_t> SignalReadPacket;

  UdpTransport* udp_transport_;
  std::unique_ptr<rtc::Thread> thread_;
};

#endif /* SRC_RTP_TRANSPORT_H_ */
