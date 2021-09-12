
#include "webrtc_transport.h"

#include <iostream>

#include <api/transport/stun.h>
#include <rtc_base/logging.h>

#include "dtls_transport.h"
#include "ice_transport.h"
#include "tools.h"

using namespace cricket;

WebrtcTransport::WebrtcTransport(const std::string& ip, const int port) {
  ice_transport_ = new IceTransport(ip, port);
  dtls_transport_ = new DtlsTransport(ice_transport_, webrtc::CryptoOptions(),
                                      rtc::SSL_PROTOCOL_DTLS_12);
}

void WebrtcTransport::Init() {
  dtls_transport_->Init();
  ice_transport_->Init();
}
