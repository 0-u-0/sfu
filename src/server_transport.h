#ifndef SERVER_TRANSPORT
#define SERVER_TRANSPORT

#include <cstddef>
#include <json.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using json = nlohmann::json;

typedef websocketpp::server<websocketpp::config::asio> server;

// pull out the type of messages sent by our config
typedef server::message_ptr message_ptr;

class ServerTransport {
 public:
  ServerTransport();
  void Init(int port);
  void Response(int id, json data);
  void Response(int id);
  void Send(const std::string& message);

  server* server_;
  websocketpp::connection_hdl handler_;
  std::unordered_map<int, std::shared_ptr<std::promise<json>>> queue_;
  std::function<void(std::string, json)> notify_callback_ = nullptr;
  std::function<void(int id, std::string, json)> request_callback_ = nullptr;

};

#endif /* SERVER_TRANSPORT */
