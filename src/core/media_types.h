#ifndef SRC_MEDIA_TYPES_H_
#define SRC_MEDIA_TYPES_H_

#include <string>

enum class MediaType { ANY, AUDIO, VIDEO, DATA };

std::string MediaTypeToString(MediaType type);

#endif /* SRC_MEDIA_TYPES_H_ */
