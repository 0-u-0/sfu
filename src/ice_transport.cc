
#include "ice_transport.h"

#include <api/transport/stun.h>
#include <rtc_base/logging.h>

#include "tools.h"

using namespace cricket;

bool ParseStunUsername(const StunMessage* stun_msg,
                       std::string* local_ufrag,
                       std::string* remote_ufrag) {
  // The packet must include a username that either begins or ends with our
  // fragment.  It should begin with our fragment if it is a request and it
  // should end with our fragment if it is a response.
  local_ufrag->clear();
  remote_ufrag->clear();
  const StunByteStringAttribute* username_attr =
      stun_msg->GetByteString(STUN_ATTR_USERNAME);
  if (username_attr == NULL)
    return false;

  // RFRAG:LFRAG
  const std::string username = username_attr->GetString();
  size_t colon_pos = username.find(':');
  if (colon_pos == std::string::npos) {
    return false;
  }

  *local_ufrag = username.substr(0, colon_pos);
  *remote_ufrag = username.substr(colon_pos + 1, username.size());
  return true;
}

void GetStunBindResponse(StunMessage* request,
                         const rtc::SocketAddress& remote_addr,
                         StunMessage* response) {
  response->SetType(STUN_BINDING_RESPONSE);
  response->SetTransactionID(request->transaction_id());

  // Tell the user the address that we received their request from.
  std::unique_ptr<StunAddressAttribute> mapped_addr;
  if (request->IsLegacy()) {
    mapped_addr = StunAttribute::CreateAddress(STUN_ATTR_MAPPED_ADDRESS);
  } else {
    mapped_addr = StunAttribute::CreateXorAddress(STUN_ATTR_XOR_MAPPED_ADDRESS);
  }
  mapped_addr->SetAddress(remote_addr);
  response->AddAttribute(std::move(mapped_addr));
}

IceTransport::IceTransport(const std::string& ip, const int port) {
  udp_transport_ = new UdpTransport(ip, port);
  ;
  local_ufrag_ = "vtucikb05exh1wax";
  local_password_ = "sufhzdkdibm2u1nml7gmo29mbsvf7i07";
}

void IceTransport::Init() {
  udp_transport_->SignalReadPacket.connect(this, &IceTransport::OnPacket);
  udp_transport_->Init();
}

void IceTransport::OnPacket(const char* data,
                            size_t size,
                            const rtc::SocketAddress& addr,
                            const int64_t timestamp) {
  if (IsStun(data, size)) {
    // Parse the request message.  If the packet is not a complete and correct
    // STUN message, then ignore it.
    std::unique_ptr<IceMessage> stun_msg(new IceMessage());
    rtc::ByteBufferReader buf(data, size);
    if (!stun_msg->Read(&buf) || (buf.Length() > 0)) {
      RTC_LOG(INFO) << "invalid!";
      return;
    }

    if (stun_msg->type() == STUN_BINDING_REQUEST) {
      RTC_LOG(INFO) << "STUN_BINDING_REQUEST";

      // Check for the presence of USERNAME and MESSAGE-INTEGRITY (if ICE)
      // first.
      // If not present, fail with a 400 Bad Request.
      if (!stun_msg->GetByteString(STUN_ATTR_USERNAME) ||
          !stun_msg->GetByteString(STUN_ATTR_MESSAGE_INTEGRITY)) {
        RTC_LOG(LS_ERROR) << "Received " << StunMethodToString(stun_msg->type())
                          << " without username/M-I from";
        // SendBindingErrorResponse(stun_msg.get(), addr,
        // STUN_ERROR_BAD_REQUEST,
        //                          STUN_ERROR_REASON_BAD_REQUEST);
        return;
      }

      // If the username is bad or unknown, fail with a 401 Unauthorized.
      std::string local_ufrag;
      std::string remote_ufrag;
      if (!ParseStunUsername(stun_msg.get(), &local_ufrag, &remote_ufrag) ||
          local_ufrag != local_ufrag_) {
        RTC_LOG(LS_ERROR) << "Received " << StunMethodToString(stun_msg->type())
                          << " with bad local username " << local_ufrag_;
        // SendBindingErrorResponse(stun_msg.get(), addr,
        // STUN_ERROR_UNAUTHORIZED,
        //                          STUN_ERROR_REASON_UNAUTHORIZED);
        return;
      }

      StunMessage response;
      // response.AddAttribute(
      //     std::make_unique<StunByteStringAttribute>(STUN_ATTR_USE_CANDIDATE));
      std::string username = remote_ufrag + ":" + local_ufrag;
      response.AddAttribute(std::make_unique<StunByteStringAttribute>(
          STUN_ATTR_USERNAME, username));
      GetStunBindResponse(stun_msg.get(), addr, &response);
      response.AddMessageIntegrity(local_password_);
      response.AddFingerprint();

      rtc::ByteBufferWriter buf;
      response.Write(&buf);
      udp_transport_->SendTo((const uint8_t*)buf.Data(), buf.Length(), addr);

      udp_transport_->SetRemoteAddress(addr);
    }
  } else if (IsDtls(data, size)) {
    SignalReadPacket(data, size);
  }
}

int IceTransport::SendPacket(const char* data,
                             size_t len,
                             const rtc::PacketOptions& options,
                             int flags) {
  RTC_LOG(INFO) << "SendPacket";
  this->udp_transport_->SendPacket((const uint8_t *)data, len);
  return 0;
}