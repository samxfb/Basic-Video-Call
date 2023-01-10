/** @file nertc_engine_video_encoder_qos_observer.h
* @brief The interface header file of expansion callback of the NERTC SDK.
* All parameter descriptions of the NERTC SDK. All string-related parameters (char *) are encoded in UTF-8.
* @copyright (c) 2022, NetEase Inc. All rights reserved.
*/

#ifndef NERTC_ENGINE_VIDEO_ENCODER_QOS_OBSERVER_H
#define NERTC_ENGINE_VIDEO_ENCODER_QOS_OBSERVER_H

#include "nertc_base_types.h"
#include "nertc_engine_defines.h"

/**
 * @namespace nertc
 * @brief namespace nertc
 */
namespace nertc
{
/** 
 * @if English
 * The SDK reports video encoder qos to the application through INERtcVideoEncoderQosObserver expansion callback interface class.
 * @endif
 * @if Chinese
 * INERtcVideoEncoderQosObserver 回调扩展接口类用于 SDK 向 App 上报视频编码qos信息。
 * @endif
 */
class INERtcVideoEncoderQosObserver 
{
public:
    virtual ~INERtcVideoEncoderQosObserver() {}
    
    /** 
     * @if English
     * The video I frame request callback.
     * @param frame The video stream type.
     * @endif
     * @if Chinese
     * 视频I帧请求回调。
     * @param video_stream_type 视频流类型。
     * @endif
     */
    virtual void onRequestSendKeyFrame(NERtcVideoStreamType video_stream_type) {}

    /** 
     * @if English
     * The video bitrate info.
     * @param bitrate_bps The video bitrate. Unit: bps。
     * @param video_stream_type The video stream type.
     * @endif
     * @if Chinese
     * 视频码率信息。
     * @param bitrate_bps 视频码率，单位: bps。
     * @param video_stream_type 视频流类型。
     * @endif
     */
    virtual void onBitrateUpdated(uint32_t bitrate_bps, NERtcVideoStreamType video_stream_type) {
        (void) bitrate_bps;
        (void) video_stream_type;
    }

    /** 
     * @if English
     * The video codec info callback.
     * @param video_codec_type The video codec type.
     * @param video_stream_type The video stream type.
     * @endif
     * @if Chinese
     * 视频编码器信息。
     * @param video_codec_type 视频编码器类型。
     * @param video_stream_type 视频流类型。
     * @endif
     */
    virtual void onVideoCodecUpdated(NERtcVideoCodecType video_codec_type, NERtcVideoStreamType video_stream_type) {
        (void) video_codec_type;
        (void) video_stream_type;
    }
};
} //namespace nertc

#endif
