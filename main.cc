#include <iostream>

#include "server_transport.h"

using namespace std;

int main(int, char **) {

  auto server_transport = new ServerTransport();
  server_transport->notify_callback_ = [](std::string method, json data) {

  };

  server_transport->request_callback_ =
      [server_transport](int id, std::string method, json params) {
        cout << "get request " << method << endl;
      };

  server_transport->Init(8800);

  std::cout << "Hello, world!\n";
}
