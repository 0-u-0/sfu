#ifndef SRC_ICE_TRANSPORT_H_
#define SRC_ICE_TRANSPORT_H_

#include <rtc_base/third_party/sigslot/sigslot.h>

#include "udp_transport.h"

// ice-lite
// https://datatracker.ietf.org/doc/html/rfc5245#section-2.7
class IceTransport : public sigslot::has_slots<> {
 public:
  IceTransport(const std::string& ip, const int port);
  void Init();
  // Attempts to send the given packet.
  // The return value is < 0 on failure. The return value in failure case is not
  // descriptive. Depending on failure cause and implementation details
  // GetError() returns an descriptive errno.h error value.
  // This mimics posix socket send() or sendto() behavior.
  // TODO(johan): Reliable, meaningful, consistent error codes for all
  // implementations would be nice.
  // TODO(johan): Remove the default argument once channel code is updated.
  int SendPacket(const char* data,
                 size_t len,
                 const rtc::PacketOptions& options,
                 int flags = 0);

  void OnPacket(const char* data,
                size_t size,
                const rtc::SocketAddress& addr,
                const int64_t timestamp);

  bool writable() { return true; };

  sigslot::signal3<const char*, size_t, const int64_t> emit_packet_;

  // variable
  std::string local_ufrag_;
  std::string local_password_;
  UdpTransport* udp_transport_;
};

#endif /* SRC_ICE_TRANSPORT_H_ */
