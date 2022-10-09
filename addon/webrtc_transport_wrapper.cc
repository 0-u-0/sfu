
#include "webrtc_transport_wrapper.h"

#include <iostream>
#include <memory>

#include "rtc_base/ssl_fingerprint.h"

#include "common/json_helper.h"

void WebrtcTransportWrapper::InitModule(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "WebrtcTransportWrapper",
      {
          InstanceMethod("Init", &WebrtcTransportWrapper::Init),
          InstanceMethod("CreateSender", &WebrtcTransportWrapper::CreateSender),
          InstanceMethod("CreateReceiver",
                         &WebrtcTransportWrapper::CreateReceiver),
          InstanceMethod("SetRemoteFingerprint",
                         &WebrtcTransportWrapper::SetRemoteFingerprint),
          InstanceMethod("Close", &WebrtcTransportWrapper::Close),

      });

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("WebrtcTransportWrapper", func);
}

WebrtcTransportWrapper::WebrtcTransportWrapper(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<WebrtcTransportWrapper>(info) {
  if (info.Length() >= 3) {
    std::string direction = info[0].ToString();
    std::string ip = info[1].ToString();
    int port = info[2].ToNumber();
    internal_ = new WebrtcTransport(direction, ip, port);
    // TODO(CC): deal with it
    this->Ref();
  } else {
    // TODO(CC): error
  }
}

WebrtcTransportWrapper::~WebrtcTransportWrapper() {}

Napi::Value WebrtcTransportWrapper::Init(const Napi::CallbackInfo& info) {
  // if (internal_) {
  //   internal_->Init();
  // }
  auto env = info.Env();

  auto cert = DtlsTransport::certificate();
  std::unique_ptr<rtc::SSLFingerprint> fingerprint =
      rtc::SSLFingerprint::CreateFromCertificate(*cert);

  internal_->Init();
  internal_->SetLocalCertificate(cert);

  Napi::Object response = Napi::Object::New(env);

  Napi::Object dtlsParameters = Napi::Object::New(env);
  Napi::Array fingerprints = Napi::Array::New(env, 1);

  Napi::Object fp = Napi::Object::New(env);

  fp.Set("algorithm", fingerprint->algorithm);
  fp.Set("value", fingerprint->GetRfc4572Fingerprint());

  fingerprints[0.] = fp;

  dtlsParameters.Set("role", "server");
  dtlsParameters.Set("fingerprints", fingerprints);

  Napi::Object iceParameters = Napi::Object::New(env);
  iceParameters.Set("iceLite", true);
  iceParameters.Set("password", "sufhzdkdibm2u1nml7gmo29mbsvf7i07");
  iceParameters.Set("usernameFragment", "vtucikb05exh1wax");

  Napi::Array iceCandidates = Napi::Array::New(env, 1);
  Napi::Object candidate = Napi::Object::New(env);
  candidate.Set("component", 1);
  candidate.Set("foundation", "udpcandidate");
  candidate.Set("ip", internal_->ip_);
  candidate.Set("port", internal_->port_);
  candidate.Set("priority", "1076302079");
  candidate.Set("protocol", "udp");
  candidate.Set("type", "host");

  iceCandidates[0.] = candidate;

  response.Set("dtlsParameters", dtlsParameters);
  response.Set("iceParameters", iceParameters);
  response.Set("iceCandidates", iceCandidates);

  return response;
}

Napi::Value WebrtcTransportWrapper::SetRemoteFingerprint(
    const Napi::CallbackInfo& info) {
  // if (internal_) {
  //   internal_->Init();
  // }
  std::string algorithm = info[0].ToString();
  std::string value = info[1].ToString();

  internal_->SetRemoteFingerprint(algorithm, value);

  return info.Env().Undefined();
}

Napi::Value WebrtcTransportWrapper::CreateSender(
    const Napi::CallbackInfo& info) {
  std::string kind = info[0].ToString();
  std::string rtp_parameter_str = info[1].ToString();
  RtpParameters rtp_parameter = nlohmann::json::parse(rtp_parameter_str);
  auto* producer = internal_->Produce(kind, rtp_parameter);
  return Napi::String::New(info.Env(), producer->id_);
}

Napi::Value WebrtcTransportWrapper::CreateReceiver(
    const Napi::CallbackInfo& info) {
  auto env = info.Env();

  WebrtcTransportWrapper* remote_transport =
      Napi::ObjectWrap<WebrtcTransportWrapper>::Unwrap(info[0].ToObject());

  std::string senderId = info[1].ToString();

  auto& sender = remote_transport->internal_->mapSender[senderId];

  auto receiver =
      internal_->CreateReceiver(sender->kind_, sender->rtp_parameter_);

  remote_transport->internal_->AddReceiverToSender(senderId, receiver);

  json rtpParameters = receiver->rtp_parameter_;

  Napi::Object result = Napi::Object::New(env);

  result.Set("id", receiver->id_);
  result.Set("rtpParameters", rtpParameters.dump());

  return result;
}

Napi::Value WebrtcTransportWrapper::Close(const Napi::CallbackInfo& info) {
  // if (internal_) {
  //   internal_->Close();
  // }

  // internal_.reset();
  this->Unref();

  return info.Env().Undefined();
}