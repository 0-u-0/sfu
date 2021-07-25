
#include "server_transport.h"

#include <iostream>

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

using namespace std;

bool validate_func_subprotocol(server *s, std::string *out, std::string accept,
                               websocketpp::connection_hdl hdl) {
  server::connection_ptr con = s->get_con_from_hdl(hdl);
  con->select_subprotocol(accept);
  return true;
}

ServerTransport::ServerTransport() { server_ = new server(); }

void ServerTransport::Response(int id, json data) {
  json requestMes;

  requestMes["id"] = id;
  requestMes["data"] = data;
  requestMes["response"] = true;
  requestMes["ok"] = true;

  const std::string mes = requestMes.dump();

  Send(mes);
}

void ServerTransport::Response(int id) {
  json requestMes;

  requestMes["id"] = id;
  requestMes["data"] = json::object();
  requestMes["response"] = true;
  requestMes["ok"] = true;

  const std::string mes = requestMes.dump();

  Send(mes);
}

void ServerTransport::Send(const std::string &message) {
  server_->send(handler_, message, websocketpp::frame::opcode::text);
}

void ServerTransport::Init(int port) {
  try {
    // Set logging settings
    server_->set_access_channels(websocketpp::log::alevel::none);
    // server_->clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize Asio
    server_->init_asio();

    std::string validate;

    // server_->set_validate_handler(
    //     bind(&validate_func_subprotocol, server_, &validate, "protoo",
    //     ::_1));

    // Register our message handler
    server_->set_message_handler(bind(
        [this](server *s, websocketpp::connection_hdl hdl, message_ptr msg) {
          //   std::cout << "on_message called with hdl: " << hdl.lock().get()
          //             << " and message: " << msg->get_payload() << std::endl;

          // check for a special command to instruct the server to stop
          // listening so it can be cleanly exited.
          if (msg->get_payload() == "stop-listening") {
            s->stop_listening();
            return;
          }

          handler_ = hdl;
          // try {
          //   s->send(hdl, msg->get_payload(), msg->get_opcode());
          // } catch (websocketpp::exception const& e) {
          //   std::cout << "Echo failed because: "
          //             << "(" << e.what() << ")" << std::endl;
          // }

          if (msg->get_opcode() == websocketpp::frame::opcode::text) {
            auto request_string = msg->get_payload();
            json request_json = json::parse(request_string);
            json id_json = request_json["id"];
            json method_json = request_json["method"];
            json params = request_json["params"];

            int id = 0;

            if (id_json.is_number_integer()) {
              id = id_json.get<int>();
            }

            if (method_json.is_string() && params.is_object()) {
              std::string method = method_json.get<std::string>();

              if (id != 0) {
                // request
                if (request_callback_) {
                  request_callback_(id, method, params);
                }
              }else {
                //notify
              }
            }

            
          }
        },
        server_, ::_1, ::_2));

    // Listen on port 9002
    server_->listen(port);

    // Start the server accept loop
    server_->start_accept();

    // Start the ASIO io_service run loop
    server_->run();
  } catch (websocketpp::exception const &e) {
    std::cout << e.what() << std::endl;
  } catch (...) {
    std::cout << "other exception" << std::endl;
  }
}