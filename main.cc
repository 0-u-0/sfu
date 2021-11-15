#include <iostream>
#include <type_traits>

#include "dtls_transport.h"
#include "ortc/rtp_parameters.h"
#include "rtc_base/event_tracer.h"
#include "rtc_base/ssl_fingerprint.h"
#include "rtp_transport.h"
#include "server_transport.h"

#include "session.h"
#include "webrtc_transport.h"

#include "json_helper.h"

int main(int, char**) {
  // rtc::tracing::SetupInternalTracer();
  // rtc::tracing::StartInternalCapture(
  //     "/Users/congchen/workspace/dugon/dugon-sfu/event_trace.log");

  auto server_transport = new ServerTransport();
  server_transport->notify_callback_ = [](std::string method, json data) {

  };

  Session* session = new Session;

  std::string ip = "127.0.0.1";
  server_transport->request_callback_ =
      [server_transport, &session, ip](int id, std::string method, json data) {
        std::cout << "get request " << method << std::endl;
        json response;
        if (method == "join") {
          std::string codec_str = R"({
      "H264-CONSTRAINED-BASELINE": {
        "channels": 1,
        "clockRate": 90000,
        "cname": null,
        "codecFullName": "H264-CONSTRAINED-BASELINE",
        "codecName": "H264",
        "dtx": false,
        "extensions": [
          {
            "id": 1,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:mid"
          },
          {
            "id": 2,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id"
          },
          {
            "id": 3,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id"
          },
          {
            "id": 4,
            "recv": true,
            "send": true,
            "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
          },
          {
            "id": 5,
            "recv": true,
            "send": true,
            "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
          },
          {
            "id": 6,
            "recv": true,
            "send": true,
            "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07"
          },
          {
            "id": 7,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:framemarking"
          },
          {
            "id": 11,
            "recv": true,
            "send": true,
            "uri": "urn:3gpp:video-orientation"
          },
          {
            "id": 12,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:toffset"
          }
        ],
        "kind": "video",
        "mid": null,
        "mux": true,
        "parameters": {
          "level-asymmetry-allowed": "1",
          "packetization-mode": "1",
          "profile-level-id": "42e01f"
        },
        "payload": 107,
        "reducedSize": true,
        "rtcpFeedback": [
          {
            "parameter": "",
            "type": "nack"
          },
          {
            "parameter": "pli",
            "type": "nack"
          },
          {
            "parameter": "fir",
            "type": "ccm"
          },
          {
            "parameter": "",
            "type": "goog-remb"
          },
          {
            "parameter": "",
            "type": "transport-cc"
          }
        ],
        "rtx": null,
        "senderPaused": false,
        "ssrc": null
      },
      "H264-MAIN": {
        "channels": 1,
        "clockRate": 90000,
        "cname": null,
        "codecFullName": "H264-MAIN",
        "codecName": "H264",
        "dtx": false,
        "extensions": [
          {
            "id": 1,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:mid"
          },
          {
            "id": 2,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id"
          },
          {
            "id": 3,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id"
          },
          {
            "id": 4,
            "recv": true,
            "send": true,
            "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
          },
          {
            "id": 5,
            "recv": true,
            "send": true,
            "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
          },
          {
            "id": 6,
            "recv": true,
            "send": true,
            "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07"
          },
          {
            "id": 7,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:framemarking"
          },
          {
            "id": 11,
            "recv": true,
            "send": true,
            "uri": "urn:3gpp:video-orientation"
          },
          {
            "id": 12,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:toffset"
          }
        ],
        "kind": "video",
        "mid": null,
        "mux": true,
        "parameters": {
          "level-asymmetry-allowed": "1",
          "packetization-mode": "1",
          "profile-level-id": "4d0032"
        },
        "payload": 105,
        "reducedSize": true,
        "rtcpFeedback": [
          {
            "parameter": "",
            "type": "nack"
          },
          {
            "parameter": "pli",
            "type": "nack"
          },
          {
            "parameter": "fir",
            "type": "ccm"
          },
          {
            "parameter": "",
            "type": "goog-remb"
          },
          {
            "parameter": "",
            "type": "transport-cc"
          }
        ],
        "rtx": null,
        "senderPaused": false,
        "ssrc": null
      },
      "VP8": {
        "channels": 1,
        "clockRate": 90000,
        "cname": null,
        "codecFullName": "VP8",
        "codecName": "VP8",
        "dtx": false,
        "extensions": [
          {
            "id": 1,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:mid"
          },
          {
            "id": 2,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id"
          },
          {
            "id": 3,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id"
          },
          {
            "id": 4,
            "recv": true,
            "send": true,
            "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
          },
          {
            "id": 5,
            "recv": true,
            "send": true,
            "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
          },
          {
            "id": 6,
            "recv": true,
            "send": true,
            "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07"
          },
          {
            "id": 7,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:framemarking"
          },
          {
            "id": 11,
            "recv": true,
            "send": true,
            "uri": "urn:3gpp:video-orientation"
          },
          {
            "id": 12,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:toffset"
          }
        ],
        "kind": "video",
        "mid": null,
        "mux": true,
        "parameters": {},
        "payload": 101,
        "reducedSize": true,
        "rtcpFeedback": [
          {
            "parameter": "",
            "type": "nack"
          },
          {
            "parameter": "pli",
            "type": "nack"
          },
          {
            "parameter": "fir",
            "type": "ccm"
          },
          {
            "parameter": "",
            "type": "goog-remb"
          },
          {
            "parameter": "",
            "type": "transport-cc"
          }
        ],
        "rtx": null,
        "senderPaused": false,
        "ssrc": null
      },
      "VP9": {
        "channels": 1,
        "clockRate": 90000,
        "cname": null,
        "codecFullName": "VP9",
        "codecName": "VP9",
        "dtx": false,
        "extensions": [
          {
            "id": 1,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:mid"
          },
          {
            "id": 2,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id"
          },
          {
            "id": 3,
            "recv": true,
            "send": false,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id"
          },
          {
            "id": 4,
            "recv": true,
            "send": true,
            "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
          },
          {
            "id": 5,
            "recv": true,
            "send": true,
            "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
          },
          {
            "id": 6,
            "recv": true,
            "send": true,
            "uri": "http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07"
          },
          {
            "id": 7,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:framemarking"
          },
          {
            "id": 11,
            "recv": true,
            "send": true,
            "uri": "urn:3gpp:video-orientation"
          },
          {
            "id": 12,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:toffset"
          }
        ],
        "kind": "video",
        "mid": null,
        "mux": true,
        "parameters": {
          "profile-id": "2"
        },
        "payload": 103,
        "reducedSize": true,
        "rtcpFeedback": [
          {
            "parameter": "",
            "type": "nack"
          },
          {
            "parameter": "pli",
            "type": "nack"
          },
          {
            "parameter": "fir",
            "type": "ccm"
          },
          {
            "parameter": "",
            "type": "goog-remb"
          },
          {
            "parameter": "",
            "type": "transport-cc"
          }
        ],
        "rtx": null,
        "senderPaused": false,
        "ssrc": null
      },
      "opus": {
        "channels": 2,
        "clockRate": 48000,
        "cname": null,
        "codecFullName": "opus",
        "codecName": "opus",
        "dtx": false,
        "extensions": [
          {
            "id": 1,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:sdes:mid"
          },
          {
            "id": 4,
            "recv": true,
            "send": true,
            "uri": "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
          },
          {
            "id": 5,
            "recv": true,
            "send": false,
            "uri": "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
          },
          {
            "id": 10,
            "recv": true,
            "send": true,
            "uri": "urn:ietf:params:rtp-hdrext:ssrc-audio-level"
          }
        ],
        "kind": "audio",
        "mid": null,
        "mux": true,
        "parameters": {},
        "payload": 100,
        "reducedSize": true,
        "rtcpFeedback": [
          {
            "parameter": "",
            "type": "transport-cc"
          }
        ],
        "rtx": null,
        "senderPaused": false,
        "ssrc": null
      }
      })";
          auto codecs = json::parse(codec_str);

          response["codecs"] = codecs;

          server_transport->Response(id, response);
        } else if (method == "createTransport") {
          // std::unique_ptr<rtc::SSLFingerprint> fingerprint =
          //       rtc::SSLFingerprint::CreateFromCertificate(*cert);
          //   if (modify_digest) {
          //     ++fingerprint->digest.MutableData()[0];
          //   }
          //   // Even if digest is verified to be incorrect, should fail
          //   asynchrnously. EXPECT_TRUE(transport->SetRemoteFingerprint(
          //       fingerprint->algorithm,
          //       reinterpret_cast<const uint8_t*>(fingerprint->digest.data()),
          //       fingerprint->digest.size()));
          std::string direction = data["direction"];

          auto cert = DtlsTransport::certificate();
          std::unique_ptr<rtc::SSLFingerprint> fingerprint =
              rtc::SSLFingerprint::CreateFromCertificate(*cert);

          WebrtcTransport* webrtc;

          int port;
          if (direction == "sendonly") {
            port = 41812;

            webrtc = session->CreateWebrtcTransport(direction, ip, port);

            // auto rtp = new RtpTransport(ip, 12312);
            // webrtc->packet_callback_list_.AddReceiver(
            //     [rtp](rtc::CopyOnWriteBuffer packet) {
            //       rtp->SendPacket(std::move(packet));
            //     });

            // rtp->SetRemoteAddress(ip, 30001);
            // rtp->Init();

            webrtc->Init();
            webrtc->SetLocalCertificate(cert);
          } else {
            port = 41813;

            webrtc = session->CreateWebrtcTransport(direction, ip, port);

            webrtc->Init();
            webrtc->SetLocalCertificate(cert);
          }

          auto dtlsParameters = json::object();
          json::array_t fingerprints = json::array();

          json fp;
          fp["algorithm"] = fingerprint->algorithm;
          fp["value"] = fingerprint->GetRfc4572Fingerprint();

          fingerprints.push_back(fp);
          dtlsParameters["fingerprints"] = fingerprints;
          dtlsParameters["role"] = "server";

          auto iceParameters = json::object();
          iceParameters["iceLite"] = true;
          iceParameters["password"] = "sufhzdkdibm2u1nml7gmo29mbsvf7i07";
          iceParameters["usernameFragment"] = "vtucikb05exh1wax";

          auto iceCandidates = json::array();
          auto candidate = json::object();

          candidate["component"] = 1;
          candidate["foundation"] = "udpcandidate";
          candidate["ip"] = ip;
          candidate["port"] = port;
          candidate["priority"] = "1076302079";
          candidate["protocol"] = "udp";
          candidate["type"] = "host";

          iceCandidates.push_back(candidate);

          response["dtlsParameters"] = dtlsParameters;
          response["iceCandidates"] = iceCandidates;
          response["iceParameters"] = iceParameters;
          response["id"] = webrtc->id_;

          server_transport->Response(id, response);
        } else if (method == "dtls") {
          std::string transportId = data["transportId"];
          auto t = session->transports[transportId];

          json dtlsParameters = data["dtlsParameters"];
          json fingerprint = dtlsParameters["fingerprint"];
          std::string algorithm = fingerprint["algorithm"];
          std::string value = fingerprint["value"];

          // fingerprint->algorithm,
          //       reinterpret_cast<const uint8_t*>(fingerprint->digest.data()),
          //       fingerprint->digest.size())
          t->SetRemoteFingerprint(algorithm, value);

          server_transport->Response(id, response);
        } else if (method == "subscribe") {
          std::string senderId = data["senderId"];
          std::string transportId = data["transportId"];
          std::string remoteTransportId = data["remoteTransportId"];

          auto remoteTransport = session->transports[remoteTransportId];
          auto& sender = remoteTransport->mapSender[senderId];

          auto t = session->transports[transportId];
          auto receiver = t->CreateReceiver(sender->rtp_parameter_);

          remoteTransport->AddReceiverToSender(senderId, receiver);

          json rtpParameters = receiver->rtp_parameter_;

          // rtpParameters["mid"] = "0";
          // rtpParameters["rtcp"] = json::parse(R"({
          //     "cname": "s5j7HzG3XQxFERZF",
          //     "reducedSize": true,
          //     "mux": true
          //   })");
          // rtpParameters["headerExtensions"] = json::parse(R"([
          //     {
          //       "uri": "urn:ietf:params:rtp-hdrext:sdes:mid",
          //       "id": 1,
          //       "encrypt": false,
          //       "parameters": {}
          //     },
          //     {
          //       "uri":
          //       "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time",
          //       "id": 4,
          //       "encrypt": false,
          //       "parameters": {}
          //     },
          //     {
          //       "uri":
          //       "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01",
          //       "id": 5,
          //       "encrypt": false,
          //       "parameters": {}
          //     },
          //     {
          //       "uri": "urn:3gpp:video-orientation",
          //       "id": 11,
          //       "encrypt": false,
          //       "parameters": {}
          //     },
          //     {
          //       "uri": "urn:ietf:params:rtp-hdrext:toffset",
          //       "id": 12,
          //       "encrypt": false,
          //       "parameters": {}
          //     }
          //   ])");

          // rtpParameters["codecs"] = json::parse(R"([
          //     {
          //       "mimeType": "video/VP8",
          //       "payloadType": 96,
          //       "clockRate": 90000,
          //       "parameters": {},
          //       "rtcpFeedback": [
          //         {
          //           "type": "transport-cc",
          //           "parameter": ""
          //         },
          //         {
          //           "type": "ccm",
          //           "parameter": "fir"
          //         },
          //         {
          //           "type": "nack",
          //           "parameter": ""
          //         },
          //         {
          //           "type": "nack",
          //           "parameter": "pli"
          //         }
          //       ]
          //     },
          //     {
          //       "mimeType": "video/rtx",
          //       "payloadType": 97,
          //       "clockRate": 90000,
          //       "parameters": {
          //         "apt": 96
          //       },
          //       "rtcpFeedback": []
          //     }
          //   ])");

          // json::array_t encodings = json::array();
          // json e;
          // e["ssrc"] = sender->ssrc_;
          // e["rtx"] = json::parse(R"({
          //     "ssrc": 104826936
          // })");
          // encodings.push_back(e);

          // rtpParameters["encodings"] = encodings;

          response["rtpParameters"] = rtpParameters;
          response["id"] = receiver->id_;

          server_transport->Response(id, response);
        } else if (method == "publish") {
          std::string transportId = data["transportId"];
          auto t = session->transports[transportId];

          RtpParameters rtpParameters;
          data["rtpParameters"].get_to(rtpParameters);

          auto* sender = t->CreateSender(rtpParameters);

          response["id"] = sender->id_;
          server_transport->Response(id, response);
          // notify
          json notify_data;

          notify_data["area"] = "jp";
          notify_data["host"] = "tokyo1";
          notify_data["mediaId"] = "foo";
          notify_data["senderId"] = sender->id_;
          notify_data["tokenId"] = "abc";
          notify_data["transportId"] = transportId;

          server_transport->Notify("publish", notify_data);
        }
      };

  server_transport->Init(8800);
}
