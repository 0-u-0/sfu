#ifndef SRC_ICE_TRANSPORT_H_
#define SRC_ICE_TRANSPORT_H_

#include "rtc_base/third_party/sigslot/sigslot.h"

#include "core/udp_transport.h"

#include "core/new_udp_transport.h"

#include "common/logger.h"

enum class IceTransportState {
  STATE_INIT,
  STATE_CONNECTING,  // Will enter this state once a connection is created
  STATE_COMPLETED,
  STATE_FAILED
};

// ice-lite, role is controlled.
// https://datatracker.ietf.org/doc/html/rfc5245#section-2.7
class IceTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  IceTransport(const std::string& ip, const int port);
  void Init();

  int SendPacket(const char* data,
                 size_t len,
                 const rtc::PacketOptions& options,
                 int flags = 0);

  void OnUdpPacket(const char* data,
                   size_t size,
                   const rtc::SocketAddress& addr,
                   const int64_t timestamp);

  void SetState(IceTransportState state);
  bool writable() { return writable_; };

  sigslot::signal3<const char*, size_t, const int64_t> on_packet_;
  sigslot::signal1<IceTransportState> emit_state_;

  // variable
  std::string local_ufrag_;
  std::string local_password_;
  rtc::Thread* thread_;

  webrtc::NewUdpTransport* udp_transport_;
  // UdpTransport* udp_transport_;

  IceTransportState state_ = IceTransportState::STATE_INIT;
  bool writable_ = false;
};

#endif /* SRC_ICE_TRANSPORT_H_ */
