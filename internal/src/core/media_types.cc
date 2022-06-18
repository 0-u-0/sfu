
#include "media_types.h"

const char kMediaTypeVideo[] = "video";
const char kMediaTypeAudio[] = "audio";
const char kMediaTypeData[] = "data";

std::string MediaTypeToString(MediaType type) {
  switch (type) {
    case MediaType::AUDIO:
      return kMediaTypeAudio;
    case MediaType::VIDEO:
      return kMediaTypeVideo;
    case MediaType::DATA:
      return kMediaTypeData;
    default:
      // Unsupported media stores the m=<mediatype> differently.
      return "";
  }
}

MediaType StringToMediaType(std::string& type) {
  if (type == kMediaTypeVideo) {
    return MediaType::VIDEO;
  } else if (type == kMediaTypeAudio) {
    return MediaType::AUDIO;
  } else if (type == kMediaTypeData) {
    return MediaType::DATA;
  } else {
    return MediaType::ANY;
  }
}