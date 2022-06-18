
#include "webrtc_transport_wrapper.h"

#include <iostream>
#include <memory>

#include "rtc_base/ssl_fingerprint.h"

void WebrtcTransportWrapper::InitModule(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "WebrtcTransportWrapper",
      {
          InstanceMethod("Init", &WebrtcTransportWrapper::Init),
          // InstanceMethod("SetCallback", &X11SourceWrapper::SetCallback),
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

  // Napi::Object iceParameters = Napi::Object::New(env);
  // iceParameters.Set("iceLite", true);

  // iceParameters["iceLite"] = true;
  // iceParameters["password"] = "sufhzdkdibm2u1nml7gmo29mbsvf7i07";
  // iceParameters["usernameFragment"] = "vtucikb05exh1wax";

  // auto iceCandidates = json::array();
  // auto candidate = json::object();

  // candidate["component"] = 1;
  // candidate["foundation"] = "udpcandidate";
  // candidate["ip"] = ip;
  // candidate["port"] = port;
  // candidate["priority"] = "1076302079";
  // candidate["protocol"] = "udp";
  // candidate["type"] = "host";

  // iceCandidates.push_back(candidate);

  response.Set("dtlsParameters", dtlsParameters);
  // response["dtlsParameters"] = dtlsParameters;
  // response["iceCandidates"] = iceCandidates;
  // response["iceParameters"] = iceParameters;
  // response["id"] = webrtc->id_;

  // server_transport->Response(id, response);

  return response;
}

Napi::Value WebrtcTransportWrapper::Close(const Napi::CallbackInfo& info) {
  // if (internal_) {
  //   internal_->Close();
  // }

  // internal_.reset();
  this->Unref();

  return info.Env().Undefined();
}