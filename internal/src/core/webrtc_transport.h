#ifndef SRC_WEBRTC_TRANSPORT_H_
#define SRC_WEBRTC_TRANSPORT_H_

#include <string>

#include <json.hpp>

#include "modules/rtp_rtcp/include/rtp_header_extension_map.h"
#include "rtc_base/callback_list.h"
#include "rtc_base/thread.h"

#include "common/logger.h"
#include "core/dtls_transport.h"
#include "core/ice_transport.h"
#include "core/receiver.h"
#include "core/rtp_demuxer.h"
#include "core/sender.h"
#include "core/srtp_transport.h"
#include "ortc/rtp_parameters.h"

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

  void OnSenderPacket(Sender*, webrtc::RtpPacketReceived&);
  void OnReceiverPacket(Receiver*, webrtc::RtpPacketReceived&);
  sigslot::signal2<Sender*, webrtc::RtpPacketReceived&> SignalReadPacket;

  Sender* CreateSender(std::string& type, RtpParameters& parameter);
  Sender* GetSender(uint32_t, std::string, std::string);

  Receiver* CreateReceiver(MediaType kind, RtpParameters& sender_parameter);
  void AddReceiverToSender(std::string senderId, Receiver* receiver);

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

  std::unordered_map<std::string, Sender*> mapSender;
  std::unordered_map<Sender*, std::unordered_set<Receiver*>> mapSenderReceiver;
};

#endif /* SRC_WEBRTC_TRANSPORT_H_ */
