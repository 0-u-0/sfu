#ifndef ADDON_WEBRTC_TRANSPORT_WRAPPER_H_
#define ADDON_WEBRTC_TRANSPORT_WRAPPER_H_

#include <napi.h>
#include <memory>

#include "core/webrtc_transport.h"

class WebrtcTransportWrapper : public Napi::ObjectWrap<WebrtcTransportWrapper> {
 public:
  static void InitModule(Napi::Env env, Napi::Object exports);
  WebrtcTransportWrapper(const Napi::CallbackInfo& info);
  ~WebrtcTransportWrapper();

  WebrtcTransport* internal_;

 private:
  Napi::Value Init(const Napi::CallbackInfo& info);
  Napi::Value CreateSender(const Napi::CallbackInfo& info);
  Napi::Value CreateReceiver(const Napi::CallbackInfo& info);
  Napi::Value SetRemoteFingerprint(const Napi::CallbackInfo& info);
  Napi::Value Close(const Napi::CallbackInfo& info);
};

#endif /* ADDON_WEBRTC_TRANSPORT_WRAPPER_H_ */
