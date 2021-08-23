#include <iostream>

#include "server_transport.h"
#include "webrtc_transport.h"

using namespace std;

int main(int, char**) {
  auto server_transport = new ServerTransport();
  server_transport->notify_callback_ = [](std::string method, json data) {

  };

  std::map<std::string, WebrtcTransport*> transports;
  std::string tid = "abc";
  std::string ip = "127.0.0.1";
  int port = 41812;
  server_transport->request_callback_ = [server_transport, &transports, tid, ip,
                                         port](int id, std::string method,
                                               json data) {
    cout << "get request " << method << endl;
    json response;
    if (method == "join") {
      std::string codec_str = "{\"H264-CONSTRAINED-BASELINE\":{\"channels\":1,\"clockRate\":90000,\"cname\":null,\"codecFullName\":\"H264-CONSTRAINED-BASELINE\",\"codecName\":\"H264\",\"dtx\":false,\"extensions\":[{\"id\":1,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:mid\"},{\"id\":2,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\"},{\"id\":3,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\"},{\"id\":4,\"recv\":true,\"send\":true,\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\"},{\"id\":5,\"recv\":true,\"send\":true,\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\"},{\"id\":6,\"recv\":true,\"send\":true,\"uri\":\"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\"},{\"id\":7,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:framemarking\"},{\"id\":11,\"recv\":true,\"send\":true,\"uri\":\"urn:3gpp:video-orientation\"},{\"id\":12,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:toffset\"}],\"kind\":\"video\",\"mid\":null,\"mux\":true,\"parameters\":{\"level-asymmetry-allowed\":\"1\",\"packetization-mode\":\"1\",\"profile-level-id\":\"42e01f\"},\"payload\":107,\"reducedSize\":true,\"rtcpFeedback\":[{\"parameter\":\"\",\"type\":\"nack\"},{\"parameter\":\"pli\",\"type\":\"nack\"},{\"parameter\":\"fir\",\"type\":\"ccm\"},{\"parameter\":\"\",\"type\":\"goog-remb\"},{\"parameter\":\"\",\"type\":\"transport-cc\"}],\"rtx\":null,\"senderPaused\":false,\"ssrc\":null},\"H264-MAIN\":{\"channels\":1,\"clockRate\":90000,\"cname\":null,\"codecFullName\":\"H264-MAIN\",\"codecName\":\"H264\",\"dtx\":false,\"extensions\":[{\"id\":1,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:mid\"},{\"id\":2,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\"},{\"id\":3,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\"},{\"id\":4,\"recv\":true,\"send\":true,\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\"},{\"id\":5,\"recv\":true,\"send\":true,\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\"},{\"id\":6,\"recv\":true,\"send\":true,\"uri\":\"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\"},{\"id\":7,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:framemarking\"},{\"id\":11,\"recv\":true,\"send\":true,\"uri\":\"urn:3gpp:video-orientation\"},{\"id\":12,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:toffset\"}],\"kind\":\"video\",\"mid\":null,\"mux\":true,\"parameters\":{\"level-asymmetry-allowed\":\"1\",\"packetization-mode\":\"1\",\"profile-level-id\":\"4d0032\"},\"payload\":105,\"reducedSize\":true,\"rtcpFeedback\":[{\"parameter\":\"\",\"type\":\"nack\"},{\"parameter\":\"pli\",\"type\":\"nack\"},{\"parameter\":\"fir\",\"type\":\"ccm\"},{\"parameter\":\"\",\"type\":\"goog-remb\"},{\"parameter\":\"\",\"type\":\"transport-cc\"}],\"rtx\":null,\"senderPaused\":false,\"ssrc\":null},\"VP8\":{\"channels\":1,\"clockRate\":90000,\"cname\":null,\"codecFullName\":\"VP8\",\"codecName\":\"VP8\",\"dtx\":false,\"extensions\":[{\"id\":1,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:mid\"},{\"id\":2,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\"},{\"id\":3,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\"},{\"id\":4,\"recv\":true,\"send\":true,\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\"},{\"id\":5,\"recv\":true,\"send\":true,\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\"},{\"id\":6,\"recv\":true,\"send\":true,\"uri\":\"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\"},{\"id\":7,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:framemarking\"},{\"id\":11,\"recv\":true,\"send\":true,\"uri\":\"urn:3gpp:video-orientation\"},{\"id\":12,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:toffset\"}],\"kind\":\"video\",\"mid\":null,\"mux\":true,\"parameters\":{},\"payload\":101,\"reducedSize\":true,\"rtcpFeedback\":[{\"parameter\":\"\",\"type\":\"nack\"},{\"parameter\":\"pli\",\"type\":\"nack\"},{\"parameter\":\"fir\",\"type\":\"ccm\"},{\"parameter\":\"\",\"type\":\"goog-remb\"},{\"parameter\":\"\",\"type\":\"transport-cc\"}],\"rtx\":null,\"senderPaused\":false,\"ssrc\":null},\"VP9\":{\"channels\":1,\"clockRate\":90000,\"cname\":null,\"codecFullName\":\"VP9\",\"codecName\":\"VP9\",\"dtx\":false,\"extensions\":[{\"id\":1,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:mid\"},{\"id\":2,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\"},{\"id\":3,\"recv\":true,\"send\":false,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\"},{\"id\":4,\"recv\":true,\"send\":true,\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\"},{\"id\":5,\"recv\":true,\"send\":true,\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\"},{\"id\":6,\"recv\":true,\"send\":true,\"uri\":\"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\"},{\"id\":7,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:framemarking\"},{\"id\":11,\"recv\":true,\"send\":true,\"uri\":\"urn:3gpp:video-orientation\"},{\"id\":12,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:toffset\"}],\"kind\":\"video\",\"mid\":null,\"mux\":true,\"parameters\":{\"profile-id\":\"2\"},\"payload\":103,\"reducedSize\":true,\"rtcpFeedback\":[{\"parameter\":\"\",\"type\":\"nack\"},{\"parameter\":\"pli\",\"type\":\"nack\"},{\"parameter\":\"fir\",\"type\":\"ccm\"},{\"parameter\":\"\",\"type\":\"goog-remb\"},{\"parameter\":\"\",\"type\":\"transport-cc\"}],\"rtx\":null,\"senderPaused\":false,\"ssrc\":null},\"opus\":{\"channels\":2,\"clockRate\":48000,\"cname\":null,\"codecFullName\":\"opus\",\"codecName\":\"opus\",\"dtx\":false,\"extensions\":[{\"id\":1,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:mid\"},{\"id\":4,\"recv\":true,\"send\":true,\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\"},{\"id\":5,\"recv\":true,\"send\":false,\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\"},{\"id\":10,\"recv\":true,\"send\":true,\"uri\":\"urn:ietf:params:rtp-hdrext:ssrc-audio-level\"}],\"kind\":\"audio\",\"mid\":null,\"mux\":true,\"parameters\":{},\"payload\":100,\"reducedSize\":true,\"rtcpFeedback\":[{\"parameter\":\"\",\"type\":\"transport-cc\"}],\"rtx\":null,\"senderPaused\":false,\"ssrc\":null}}";
      auto codecs = json::parse(codec_str);

      response["codecs"] = codecs;

      server_transport->Response(id, response);
    } else if (method == "createTransport") {
      auto webrtc = new WebrtcTransport(ip, port);
      webrtc->Init();
      transports[tid] = webrtc;

      auto dtlsParameters = json::object();
      dtlsParameters["algorithm"] = "sha-256";
      dtlsParameters["setup"] = "active";
      dtlsParameters["value"] = "94:99:57:85:BE:0F:45:22:E2:53:6C:34:03:71:B8:F2:7C:A7:9F:0A:C9:94:69:77:D1:4D:B7:34:D2:41:A3:BC";

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
      candidate["transport"] = "udp";
      candidate["type"] = "host";

      iceCandidates.push_back(candidate);

      response["dtlsParameters"] = dtlsParameters;
      response["iceCandidates"] = iceCandidates;
      response["iceParameters"] = iceParameters;
      response["id"] = tid;


      server_transport->Response(id, response);
    }
  };

  server_transport->Init(8800);

  std::cout << "Hello, world!\n";
}