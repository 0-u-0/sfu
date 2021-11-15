#ifndef SRC_JSON_HELPER_H_
#define SRC_JSON_HELPER_H_

#include "api/media_types.h"
#include "api/rtp_parameters.h"

#include <json.hpp>

namespace nlohmann {

// RtcpParameters
template <>
struct adl_serializer<webrtc::RtcpParameters> {
  static void to_json(json& j, const webrtc::RtcpParameters& parameter) {
    j = json{
        {"cname", parameter.cname},
    };
  };

  static void from_json(const json& j, webrtc::RtcpParameters& parameter) {
    j.at("cname").get_to(parameter.cname);
  };
};

// RtpExtension
template <>
struct adl_serializer<webrtc::RtpExtension> {
  static void to_json(json& j, const webrtc::RtpExtension& parameter) {
    j = json{
        {"uri", parameter.uri},
    };
  };

  static void from_json(const json& j, webrtc::RtpExtension& parameter) {
    j.at("uri").get_to(parameter.uri);
  };
};

// RtpEncodingParameters
template <>
struct adl_serializer<webrtc::RtpEncodingParameters> {
  static void to_json(json& j, const webrtc::RtpEncodingParameters& parameter) {
    j = json{};
    if (parameter.ssrc) {
      j["ssrc"] = parameter.ssrc.value();
    }
  };

  static void from_json(const json& j,
                        webrtc::RtpEncodingParameters& parameter) {
    if (j.contains("ssrc")) {
      parameter.ssrc = j.at("ssrc").get<int>();
    }
  };
};

// RtcpFeedback
template <>
struct adl_serializer<webrtc::RtcpFeedback> {
  static void to_json(json& j, const webrtc::RtcpFeedback& feedback) {
    j = json{};
  };

  static void from_json(const json& j, webrtc::RtcpFeedback& feedback){
      // j.at("payloadType").get_to(parameter.payload_type);
  };
};

// RtpCodecParameters
template <>
struct adl_serializer<webrtc::RtpCodecParameters> {
  static void to_json(json& j, const webrtc::RtpCodecParameters& parameter) {
    j = json{
        {"payloadType", parameter.payload_type},
        {"clockRate", parameter.clock_rate.value()},
        {"parameters", parameter.parameters},
        {"mimeType", parameter.mime_type()},
        {"rtcpFeedback", parameter.rtcp_feedback},
    };
  };

  static void from_json(const json& j, webrtc::RtpCodecParameters& parameter) {
    j.at("payloadType").get_to(parameter.payload_type);
    j.at("rtcpFeedback").get_to(parameter.rtcp_feedback);

    parameter.clock_rate = j.at("clockRate").get<int>();

    // TODO(CC): parse parameters
    // j.at("parameters").get_to(parameter.parameters);

    std::string mime_type = j.at("mimeType").get<std::string>();

    if (mime_type.find("audio") != std::string::npos) {
      parameter.kind = cricket::MEDIA_TYPE_AUDIO;
    } else if (mime_type.find("video") != std::string::npos) {
      parameter.kind = cricket::MEDIA_TYPE_VIDEO;
    } else {
      parameter.kind = cricket::MEDIA_TYPE_UNSUPPORTED;
    }

    parameter.name = mime_type.substr(6);  //  5 + 1
    // mime_type.
  };
};

// RtpParameters
template <>
struct adl_serializer<webrtc::RtpParameters> {
  static void to_json(json& j, const webrtc::RtpParameters& parameter) {
    j = json{
        {"mid", parameter.mid},
        {"rtcp", parameter.rtcp},
        {"headerExtensions", parameter.header_extensions},
        {"encodings", parameter.encodings},
        {"codecs", parameter.codecs},
    };
  };

  // rtp_parameters.codecs
  static void from_json(const json& j, webrtc::RtpParameters& parameter) {
    j.at("mid").get_to(parameter.mid);
    j.at("rtcp").get_to(parameter.rtcp);
    j.at("headerExtensions").get_to(parameter.header_extensions);
    j.at("encodings").get_to(parameter.encodings);
    j.at("codecs").get_to(parameter.codecs);
  };
};

}  // namespace nlohmann

#endif /* SRC_JSON_HELPER_H_ */
