#ifndef SRC_JSON_HELPER_H_
#define SRC_JSON_HELPER_H_

#include "api/media_types.h"
#include "api/rtp_parameters.h"

#include "ortc/rtp_parameters.h"

#include <any>
#include <json.hpp>
#include <map>

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
        {"id", parameter.id},
    };
  };

  static void from_json(const json& j, webrtc::RtpExtension& parameter) {
    j.at("uri").get_to(parameter.uri);
    j.at("id").get_to(parameter.id);
  };
};

template <>
struct adl_serializer<webrtc::RtpRtxParameters> {
  static void to_json(json& j, const webrtc::RtpRtxParameters& parameter) {
    j = json{};
    if (parameter.ssrc) {
      j["ssrc"] = parameter.ssrc.value();
    }
  };

  static void from_json(const json& j, webrtc::RtpRtxParameters& parameter) {
    if (j.contains("ssrc")) {
      parameter.ssrc = j.at("ssrc").get<int>();
    }
  };
};

// RtpEncodingParameters
template <>
struct adl_serializer<RtpEncodingParameters> {
  static void to_json(json& j, const RtpEncodingParameters& parameter) {
    j = json{};
    if (parameter.ssrc) {
      j["ssrc"] = parameter.ssrc.value();
    }
    if (parameter.rtx) {
      j["rtx"] = parameter.rtx.value();
    }
  };

  static void from_json(const json& j, RtpEncodingParameters& parameter) {
    if (j.contains("ssrc")) {
      parameter.ssrc = j.at("ssrc").get<int>();
    }
    if (j.contains("rtx")) {
      parameter.rtx = j.at("rtx").get<webrtc::RtpRtxParameters>();
    }
  };
};

// RtcpFeedback
template <>
struct adl_serializer<webrtc::RtcpFeedback> {
  static void to_json(json& j, const webrtc::RtcpFeedback& feedback) {
    j = json{};
    if (feedback.type == webrtc::RtcpFeedbackType::NACK) {
      j["type"] = "nack";
    } else if (feedback.type == webrtc::RtcpFeedbackType::CCM) {
      j["type"] = "ccm";
    } else if (feedback.type == webrtc::RtcpFeedbackType::TRANSPORT_CC) {
      j["type"] = "transport-cc";
    } else if (feedback.type == webrtc::RtcpFeedbackType::REMB) {
      j["type"] = "goog-remb";
    }

    if (feedback.message_type.has_value()) {
      if (feedback.message_type.value() ==
          webrtc::RtcpFeedbackMessageType::FIR) {
        j["parameter"] = "fir";
      } else if (feedback.message_type.value() ==
                 webrtc::RtcpFeedbackMessageType::PLI) {
        j["parameter"] = "pli";
      } else if (feedback.message_type.value() ==
                 webrtc::RtcpFeedbackMessageType::GENERIC_NACK) {
        j["parameter"] = "";
      }
    } else {
      j["parameter"] = "";
    }
  };

  static void from_json(const json& j, webrtc::RtcpFeedback& feedback) {
    std::string type = j.at("type").get<std::string>();
    std::string parameter = j.at("parameter").get<std::string>();

    if (type == "nack") {
      feedback.type = webrtc::RtcpFeedbackType::NACK;
    } else if (type == "ccm") {
      feedback.type = webrtc::RtcpFeedbackType::CCM;
    } else if (type == "transport-cc") {
      feedback.type = webrtc::RtcpFeedbackType::TRANSPORT_CC;
    } else if (type == "goog-remb") {
      feedback.type = webrtc::RtcpFeedbackType::REMB;
    }

    if (parameter.empty()) {
    } else if (parameter == "pli") {
      feedback.message_type = webrtc::RtcpFeedbackMessageType::PLI;
    } else if (parameter == "fir") {
      feedback.message_type = webrtc::RtcpFeedbackMessageType::FIR;
    }

    // j.at("type").get_to();
  };
};

// RtpCodecParameters
template <>
struct adl_serializer<RtpCodecParameters> {
  static void to_json(json& j, const RtpCodecParameters& parameter) {
    j = json{
        {"payloadType", parameter.payload_type},
        {"clockRate", parameter.clock_rate.value()},
        {"parameters", parameter.parameters},
        {"mimeType", parameter.mime_type()},
        {"rtcpFeedback", parameter.rtcp_feedback},
    };
  };

  static void from_json(const json& j, RtpCodecParameters& parameter) {
    j.at("payloadType").get_to(parameter.payload_type);
    j.at("rtcpFeedback").get_to(parameter.rtcp_feedback);

    parameter.clock_rate = j.at("clockRate").get<int>();

    // TODO(CC): parse parameters
    j.at("parameters").get<std::map<std::string, json>>();
    // parameter.parameters;

    // mime_type
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
struct adl_serializer<RtpParameters> {
  static void to_json(json& j, const RtpParameters& parameter) {
    j = json{
        {"mid", parameter.mid},
        {"rtcp", parameter.rtcp},
        {"headerExtensions", parameter.header_extensions},
        {"encodings", parameter.encodings},
        {"codecs", parameter.codecs},
    };
  };

  // rtp_parameters.codecs
  static void from_json(const json& j, RtpParameters& parameter) {
    j.at("mid").get_to(parameter.mid);
    j.at("rtcp").get_to(parameter.rtcp);
    j.at("headerExtensions").get_to(parameter.header_extensions);
    j.at("encodings").get_to(parameter.encodings);
    j.at("codecs").get_to(parameter.codecs);
  };
};

}  // namespace nlohmann

#endif /* SRC_JSON_HELPER_H_ */
