#include <napi.h>

#include "webrtc_transport_wrapper.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  WebrtcTransportWrapper::InitModule(env, exports);

  return exports;
}

NODE_API_MODULE(addon, InitAll)