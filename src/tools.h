#ifndef SRC_TOOLS_H_
#define SRC_TOOLS_H_

#include <cstddef>

bool IsStun(const char* data, size_t size);
bool IsRtp(const char* data, size_t size);
bool IsRtcp(const char* data, size_t size);
bool IsDtls(const char* data, size_t size);

#endif /* SRC_TOOLS_H_ */
