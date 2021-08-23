
#include "webrtc_transport.h"

#include <iostream>

WebrtcTransport::WebrtcTransport(const std::string& ip, const int port) {
  udp_transport = new UdpTransport(ip, port);
  udp_transport->data_callback_ = [this](const char* data, size_t size) {
    Receive(data, size);
  };
}

void WebrtcTransport::Init(){
  udp_transport->Init();
}

void WebrtcTransport::Receive(const char *data, size_t size){
  std::cout << "data!" << std::endl;
}