
#include "receiver.h"

#include "absl/types/optional.h"
#include "api/video_codecs/video_codec.h"
#include "modules/rtp_rtcp/source/create_video_rtp_depacketizer.h"
#include "modules/video_coding/packet_buffer.h"
#include "rtc_base/helpers.h"

DEFINE_LOGGER(Receiver, "Receiver");

Receiver::Receiver(MediaType kind, RtpParameters& parameter)
    : id_(rtc::CreateRandomUuid()) {
  kind_ = kind;
  rtp_parameter_ = parameter;
  rtp_parameter_.encodings[0].ssrc = 11111111;

  codec_type_ = webrtc::PayloadStringToCodecType(rtp_parameter_.codecs[0].name);

  if (kind_ == MediaType::VIDEO) {
    video_rtp_depacketizer_ = webrtc::CreateVideoRtpDepacketizer(codec_type_);
  }
}

void Receiver::OnSenderPacket(Sender*, webrtc::RtpPacketReceived& rtp_packet) {
  ILOG("receiver on packet {} {}",
       webrtc::CodecTypeToPayloadString(codec_type_),
       rtp_packet.payload_size());

  if (sync_required_) {
    if (kind_ == MediaType::AUDIO) {
      // TODO(CC): same code
      seq_manager.Sync(rtp_packet.SequenceNumber() -
                       1);  // TODO(CC): why minus 1?
      sync_required_ = false;
    } else if (kind_ == MediaType::VIDEO) {
      absl::optional<webrtc::VideoRtpDepacketizer::ParsedRtpPayload>
          parsed_payload =
              video_rtp_depacketizer_->Parse(rtp_packet.PayloadBuffer());

      if (parsed_payload == absl::nullopt) {
        WLOG("Failed parsing payload.");
        return;
      }

      if (!parsed_payload.has_value()) {
        WLOG("No Value");
        return;
      }

      webrtc::VideoRtpDepacketizer::ParsedRtpPayload real =
          parsed_payload.value();

      real.video_payload.Clear();

      // auto video_packet =
      //     std::make_unique<webrtc::video_coding::PacketBuffer::Packet>(
      //         rtp_packet, parsed_payload->video_header);

      // if (video_packet->video_header.frame_type ==
      //     webrtc::VideoFrameType::kVideoFrameKey) {
      //   // TODO(CC): same code
      //   seq_manager.Sync(rtp_packet.SequenceNumber() -
      //                    1);  // TODO(CC): why minus 1?
      //   sync_required_ = false;
      //   ILOG("keyframe");
      // } else {
      //   return;
      // }
    }
  }

  rtp_packet.SetSsrc(rtp_parameter_.encodings[0].ssrc.value());
  SignalReadPacket(this, rtp_packet);
}
