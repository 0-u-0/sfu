#ifndef SRC_RTP_TRANSPORT_H_
#define SRC_RTP_TRANSPORT_H_

#include <rtc_base/thread.h>
#include <rtc_base/copy_on_write_buffer.h>

#include "udp_transport.h"

class RtpTransport {
 public:
  RtpTransport(const std::string& ip, const int port);
  void Init();
  void SetRemoteAddress(const std::string& ip, const int port);
  void SendPacket(rtc::CopyOnWriteBuffer packet);

  UdpTransport* udp_transport_;
  std::unique_ptr<rtc::Thread> thread_;
};

#endif /* SRC_RTP_TRANSPORT_H_ */
