
#include "core/socket_pool.h"

#include <iostream>
#include <thread>
#include "core/new_udp_transport.h"
#include "rtc_base/thread.h"

namespace webrtc {
std::vector<std::unique_ptr<rtc::Thread>> SocketPool::thread_pool_;

int SocketPool::current_index_ = 0;
int SocketPool::pool_size_ = 0;
std::atomic_bool SocketPool::inited_{false};
std::mutex SocketPool::udp_mutex_;

void SocketPool::Init() {
  if (inited_) {
    return;
  }
  inited_ = true;

  const auto processor_count = std::thread::hardware_concurrency();
  pool_size_ = processor_count * 2;

  for (int i = 0; i < pool_size_; ++i) {
    auto thread = rtc::Thread::CreateWithSocketServer();
    thread->SetName("SocketThread" + std::to_string(i), nullptr);
    thread->Start();

    thread_pool_.push_back(std::move(thread));
  }
}

void SocketPool::Unint() {}

NewUdpTransport* SocketPool::AllocateUdp(const std::string& ip, int port) {
  std::lock_guard<std::mutex> guard(udp_mutex_);

  auto current_thread = thread_pool_[current_index_].get();
  webrtc::NewUdpTransport* udp =
      current_thread->Invoke<webrtc::NewUdpTransport*>(
          RTC_FROM_HERE,
          [ip, port]() { return new webrtc::NewUdpTransport(ip, port); });

  if (current_index_ + 1 < pool_size_) {
    current_index_ += 1;
  } else {
    current_index_ = 0;
  }

  return udp;
}

}  // namespace webrtc