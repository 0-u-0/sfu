#ifndef SRC_ORTC_RTP_PARAMETERS_H_
#define SRC_ORTC_RTP_PARAMETERS_H_

#include "api/rtp_parameters.h"

#include <optional>

struct RtpEncodingParameters : webrtc::RtpEncodingParameters {
  std::optional<webrtc::RtpRtxParameters> rtx;
};
struct RtpCodecParameters : webrtc::RtpCodecParameters {};
struct RtpParameters : webrtc::RtpParameters {
  std::vector<RtpCodecParameters> codecs;
  std::vector<RtpEncodingParameters> encodings;
};
#endif /* SRC_ORTC_RTP_PARAMETERS_H_ */
