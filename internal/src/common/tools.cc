

#include "tools.h"

#include <api/transport/stun.h>

bool IsStun(const char* data, size_t size) {
  int types[] = {cricket::GOOG_PING_REQUEST, cricket::GOOG_PING_RESPONSE,
                 cricket::GOOG_PING_ERROR_RESPONSE};
  if (!cricket::StunMessage::IsStunMethod(types, data, size) &&
      !cricket::StunMessage::ValidateFingerprint(data, size)) {
    return false;
  }
  return true;
}

bool IsDtls(const char* data, size_t len) {
  // Minimum DTLS record length is 13 bytes &
  // https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
  return ((len >= 13) && (data[0] > 19 && data[0] < 64));
}
