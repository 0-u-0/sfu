
#include "core/socket_pool.h"

#include <iostream>
#include <thread>

#include "rtc_base/thread.h"

namespace webrtc {
DEFINE_LOGGER(SocketPool, "SocketPool");

SocketPool* SocketPool::pool_{nullptr};

// std::vector<std::unique_ptr<rtc::Thread>> SocketPool::thread_pool_;

// int SocketPool::current_index_ = 0;
// int SocketPool::pool_size_ = 0;

std::mutex SocketPool::udp_mutex_;

void SocketPool::Init() {
  std::lock_guard<std::mutex> guard(udp_mutex_);

  if (pool_ != nullptr) {
    return;
  }

  ILOG("Init");

  pool_ = new SocketPool();

  const auto processor_count = std::thread::hardware_concurrency();
  pool_->pool_size_ = processor_count;

  for (int i = 0; i < pool_->pool_size_; ++i) {
    auto thread = rtc::Thread::CreateWithSocketServer();
    thread->SetName("SocketThread" + std::to_string(i), nullptr);
    thread->Start();

    pool_->thread_pool_.push_back(std::move(thread));
  }
}

void SocketPool::Unint() {}

UdpTransport* SocketPool::AllocateUdp(const std::string& ip, int port) {
  rtc::Thread* current_thread;

  {
    std::lock_guard<std::mutex> guard(udp_mutex_);
    if (pool_ != nullptr) {
      ILOG("Allocate udp index {}, {} {}", pool_->current_index_, ip, port);
      current_thread = pool_->thread_pool_[pool_->current_index_].get();

      if (pool_->current_index_ + 1 < pool_->pool_size_) {
        pool_->current_index_ += 1;
      } else {
        pool_->current_index_ = 0;
      }
    } else {
      return nullptr;
    }
  }

  webrtc::UdpTransport* udp = current_thread->Invoke<webrtc::UdpTransport*>(
      RTC_FROM_HERE,
      [ip, port]() { return new webrtc::UdpTransport(ip, port); });
  return udp;
}

}  // namespace webrtc