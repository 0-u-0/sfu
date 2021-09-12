#ifndef SRC_ICE_TRANSPORT_H_
#define SRC_ICE_TRANSPORT_H_

#include "rtc_base/async_packet_socket.h"

class IceTransport {
 public:

 
  bool writable() { return true; };

  // Attempts to send the given packet.
  // The return value is < 0 on failure. The return value in failure case is not
  // descriptive. Depending on failure cause and implementation details
  // GetError() returns an descriptive errno.h error value.
  // This mimics posix socket send() or sendto() behavior.
  // TODO(johan): Reliable, meaningful, consistent error codes for all
  // implementations would be nice.
  // TODO(johan): Remove the default argument once channel code is updated.
  virtual int SendPacket(const char* data,
                         size_t len,
                         const rtc::PacketOptions& options,
                         int flags = 0) = 0;
};

#endif /* SRC_ICE_TRANSPORT_H_ */
