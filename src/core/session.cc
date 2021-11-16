
#include "session.h"

#include <rtc_base/logging.h>

WebrtcTransport* Session::CreateWebrtcTransport(const std::string& direction,
                                                const std::string& ip,
                                                const int port) {
  auto webrtc = new WebrtcTransport(direction, ip, port);
  webrtc->SignalReadPacket.connect(this, &Session::OnSenderPacket);
  transports[webrtc->id_] = webrtc;
  return webrtc;
}

void Session::OnSenderPacket(Sender* sender,
                             webrtc::RtpPacketReceived& packet) {
  RTC_LOG(INFO) << "session sender packet";
}
