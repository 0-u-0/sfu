#ifndef SRC_WEBRTC_TRANSPORT_H_
#define SRC_WEBRTC_TRANSPORT_H_

#include <string>

#include "modules/rtp_rtcp/include/rtp_header_extension_map.h"
#include "rtc_base/callback_list.h"
#include "rtc_base/thread.h"

#include "common/logger.h"
#include "core/consumer.h"
#include "core/dtls_transport.h"
#include "core/ice_transport.h"
#include "core/producer.h"
#include "core/rtp_demuxer.h"
#include "core/srtp_transport.h"
#include "ortc/rtp_parameters.h"

#include "third_party/nlohmann/json.hpp"

using json = nlohmann::json;

class WebrtcTransport : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  WebrtcTransport(const std::string& direction,
                  const std::string& ip,
                  const int port);

  void Init();
  bool SetLocalCertificate(
      const rtc::scoped_refptr<rtc::RTCCertificate>& certificate);
  rtc::scoped_refptr<rtc::RTCCertificate> GetLocalCertificate() const;

  bool SetRemoteFingerprint(const std::string& algorithm,
                            const std::string& fingerprint);

  void OnPacket(rtc::CopyOnWriteBuffer& packet);
  void HandleRtcp(rtc::ArrayView<const uint8_t> packet);
  void SendPacket(const char* data, size_t size, const int64_t timestamp);
  void SendPacket(webrtc::RtpPacketReceived& packet);

  void OnSenderPacket(Producer*, webrtc::RtpPacketReceived&);
  void OnReceiverPacket(Consumer*, webrtc::RtpPacketReceived&);
  sigslot::signal2<Producer*, webrtc::RtpPacketReceived&> SignalReadPacket;

  Producer* Produce(std::string& type, RtpParameters& parameter);
  Producer* GetProducer(uint32_t, std::string, std::string);

  Consumer* CreateReceiver(MediaType kind, RtpParameters& sender_parameter);
  void AddReceiverToSender(std::string senderId, Consumer* receiver);

  const std::string id_;

  std::string ip_;
  int port_;

  std::unique_ptr<rtc::Thread> thread_;

  std::string direction_;
  DtlsTransport* dtls_transport_;
  IceTransport* ice_transport_;
  SrtpTransport* srtp_transport_;

  webrtc::CallbackList<rtc::CopyOnWriteBuffer> packet_callback_list_;

  webrtc::RtpHeaderExtensionMap rtp_header_extensions_;
  RtpDemuxer* rtp_demuxer_;

  std::unordered_map<std::string, Producer*> mapSender;
  std::unordered_map<Producer*, std::unordered_set<Consumer*>>
      mapSenderReceiver;
};

#endif /* SRC_WEBRTC_TRANSPORT_H_ */
