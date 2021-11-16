
// #include "json_helper.h"
// #include "ortc/rtp_parameters.h"

#include "log4cxx_sink.h"

#include "logger.h"
#include "rtc_base/logging.h"
// #include "rtc_base/logging.h"

// void to_json(json& j, const webrtc::RtpParameters& parameter) {
//   j = json{{"mid", parameter.mid}};
// }

// void from_json(const json& j, webrtc::RtpParameters& parameter) {
//   j.at("mid").get_to(parameter.mid);
// }

// using json = nlohmann::json;

// void jsonTest() {
//   json rtp_json = R"({
//       "codecs": [
//         {
//           "mimeType": "video/VP8",
//           "payloadType": 96,
//           "clockRate": 90000,
//           "parameters": {},
//           "rtcpFeedback": [
//             {
//               "type": "goog-remb",
//               "parameter": ""
//             },
//             {
//               "type": "transport-cc",
//               "parameter": ""
//             },
//             {
//               "type": "ccm",
//               "parameter": "fir"
//             },
//             {
//               "type": "nack",
//               "parameter": ""
//             },
//             {
//               "type": "nack",
//               "parameter": "pli"
//             }
//           ]
//         },
//         {
//           "mimeType": "video/rtx",
//           "payloadType": 97,
//           "clockRate": 90000,
//           "parameters": {
//             "apt": 96
//           },
//           "rtcpFeedback": []
//         }
//       ],
//       "headerExtensions": [
//         {
//           "uri": "urn:ietf:params:rtp-hdrext:sdes:mid",
//           "id": 4,
//           "encrypt": false,
//           "parameters": {}
//         },
//         {
//           "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id",
//           "id": 5,
//           "encrypt": false,
//           "parameters": {}
//         },
//         {
//           "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id",
//           "id": 6,
//           "encrypt": false,
//           "parameters": {}
//         },
//         {
//           "uri":
//           "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time", "id":
//           2, "encrypt": false, "parameters": {}
//         },
//         {
//           "uri":
//           "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
//           "id": 3,
//           "encrypt": false,
//           "parameters": {}
//         },
//         {
//           "uri": "urn:3gpp:video-orientation",
//           "id": 13,
//           "encrypt": false,
//           "parameters": {}
//         },
//         {
//           "uri": "urn:ietf:params:rtp-hdrext:toffset",
//           "id": 14,
//           "encrypt": false,
//           "parameters": {}
//         }
//       ],
//       "encodings": [
//         {
//           "ssrc": 3376946633,
//           "rtx": {
//             "ssrc": 3623842814
//           },
//           "dtx": false
//         }
//       ],
//       "rtcp": {
//         "cname": "UXhJUhWQJg5+SZ/g",
//         "reducedSize": true
//       },
//       "mid": "0"
//     })"_json;
//   // auto parameter = jsonToRtpParameters(json_str);

//   // create a person
//   json j = rtp_json.get<RtpParameters>();
//   // conversion: person -> json

//   std::cout << j << std::endl;
// }

int main() {
  // log4cxx::LoggerPtr logger;
  // logger.

  LOG_CONFIGURATION("/Users/congchen/workspace/dugon/sfu/log4cxx.properties");

  rtc::LogMessage::SetLogToStderr(false);
  Log4cxxSink log4;
  rtc::LogMessage::AddLogToStream(&log4, rtc::LS_VERBOSE);

  RTC_LOG(LS_VERBOSE) << "debug";
  RTC_LOG(INFO) << "debug";
  RTC_LOG(WARNING) << "debug";
  RTC_LOG(LS_ERROR) << "debug";
  return 0;
}