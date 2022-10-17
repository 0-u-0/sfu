#ifndef SRC_CORE_SOCKET_POOL_H_
#define SRC_CORE_SOCKET_POOL_H_

#include <atomic>
#include <mutex>

#include "rtc_base/thread.h"

#include "common/logger.h"
#include "core/udp_transport.h"

namespace webrtc {
class SocketPool {
 public:
  DECLARE_LOGGER();

  //  rtc::Thread* thread_;
  static void Init();
  static void Unint();

  static UdpTransport* AllocateUdp(const std::string& ip, int port);

  static SocketPool* pool_;

  static std::mutex udp_mutex_;

  std::vector<std::unique_ptr<rtc::Thread>> thread_pool_;
  int pool_size_ = 0;
  int current_index_ = 0;
};
}  // namespace webrtc

#endif /* SRC_CORE_SOCKET_POOL_H_ */
