#ifndef SRC_CORE_SOCKET_POOL_H_
#define SRC_CORE_SOCKET_POOL_H_

#include <atomic>
#include <mutex>

#include "core/new_udp_transport.h"
#include "rtc_base/thread.h"

namespace webrtc {
class SocketPool {
 public:
  //  rtc::Thread* thread_;
  static void Init();
  static void Unint();
  static NewUdpTransport* AllocateUdp(const std::string& ip, int port);

  static std::vector<std::unique_ptr<rtc::Thread>> thread_pool_;
  static int pool_size_;
  static int current_index_;
  static std::mutex udp_mutex_;
  static std::atomic_bool inited_;
};
}  // namespace webrtc

#endif /* SRC_CORE_SOCKET_POOL_H_ */
