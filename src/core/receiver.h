#ifndef SRC_RECEIVER_H_
#define SRC_RECEIVER_H_

#include <string>

#include "modules/rtp_rtcp/source/video_rtp_depacketizer.h"

#include "common/logger.h"
#include "sender.h"
#include "seq_manager.h"

class Receiver : public sigslot::has_slots<> {
  DECLARE_LOGGER();

 public:
  Receiver(MediaType kind, RtpParameters& parameter);
  void OnSenderPacket(Sender*, webrtc::RtpPacketReceived&);

  sigslot::signal2<Receiver*, webrtc::RtpPacketReceived&> SignalReadPacket;

  const std::string id_;
  RtpParameters rtp_parameter_;
  MediaType kind_;
  webrtc::VideoCodecType codec_type_;
  SeqManager<uint16_t> seq_manager;
  std::unique_ptr<webrtc::VideoRtpDepacketizer> video_rtp_depacketizer_;
  bool sync_required_ = true;
};

#endif /* SRC_RECEIVER_H_ */
