#pragma once

//#define CONFIG_FILE_PLAYER_TEST_ENABLED
//#define CONFIG_MONKEY_TEST_ENABLED

// https://console.shengwang.cn/
#define CONFIG_AGORA_APP_ID "24cbe3f***************85f3ff03" // Please replace with your own APP ID

// Agora Server URL
#define CONFIG_SERVER_URL "https://api.sd-rtn.com/agoralink/cn/api"

/**
 * @brief The data type of the video, refer to the definition of ago_av_data_type_e
 *
 */
#define SEND_VIDEO_DATA_TYPE (1) // 1 for H264 and 2 for JPEG, 3 for H265

#if (SEND_VIDEO_DATA_TYPE == 1)
#include "h264_test_data_352x288.h"
#define CONFIG_SEND_H264_FRAMES
#define CONFIG_SEND_FRAME_RATE (25)
#elif (SEND_VIDEO_DATA_TYPE == 2)
#include "jpeg_test_data_640x480.h"
#define CONFIG_SEND_JPEG_FRAMES
#define CONFIG_SEND_FRAME_RATE (5)
#elif (SEND_VIDEO_DATA_TYPE == 3)
#include "h265_test_data_360x640.h"
#define CONFIG_SEND_H265_FRAMES
#define CONFIG_SEND_FRAME_RATE (25)
#else
// to be added
#endif

/**
 * Note: if the data type of the audio to send is not PCM, such as G711U,
 * and the shoule be set INTERNAL_AUDIO_ENC_TYPE to be 0 (Disable).
 */
#define SEND_AUDIO_DATA_TYPE (10) // 10 for PCM, 13 for G711U

/**
 * @brief The internal audio encoder codec type
 *
 * The SDK supports encode the PCM data before sending.
 * The codec type of encoder refer to the definition of ago_audio_codec_type_e
 * 0 -> Disable encoder
 * 1 -> OPUS
 * 2 -> G722
 * 3 -> G711A
 * 4 -> G711U
 *
 * Note: The Agora OSS doesn't support OPUS yet.
 */
#define INTERNAL_AUDIO_ENC_TYPE (4)

#if (INTERNAL_AUDIO_ENC_TYPE == 0)
#define CONFIG_PCM_SAMPLE_RATE (16000)
#define CONFIG_PCM_CHANNEL_NUM (1)
#elif (INTERNAL_AUDIO_ENC_TYPE == 1)
#define CONFIG_PCM_SAMPLE_RATE (16000)
#define CONFIG_PCM_CHANNEL_NUM (1)
#elif (INTERNAL_AUDIO_ENC_TYPE == 2)
#define CONFIG_PCM_SAMPLE_RATE (16000)
#define CONFIG_PCM_CHANNEL_NUM (1)
#elif (INTERNAL_AUDIO_ENC_TYPE == 3)
#define CONFIG_PCM_SAMPLE_RATE (8000)
#define CONFIG_PCM_CHANNEL_NUM (1)
#elif (INTERNAL_AUDIO_ENC_TYPE == 4)
#define CONFIG_PCM_SAMPLE_RATE (8000)
#define CONFIG_PCM_CHANNEL_NUM (1)
#endif

// TODO: If you changed the audio test data, you should also update the sample rate, the channel number matched your own data.
#if (SEND_AUDIO_DATA_TYPE == 10)
#include "pcm_test_data_16K_16bit_1ch_5s.h"
#define AUDIO_DATA pcm_test_data
#define AUDIO_FRAME_DURATION_MS (20)
#define AUDIO_FRAME_LEN \
        (AUDIO_FRAME_DURATION_MS * sizeof(int16_t) * CONFIG_PCM_CHANNEL_NUM * (CONFIG_PCM_SAMPLE_RATE / 1000))
#elif (SEND_AUDIO_DATA_TYPE == 13)
#include "g711u_test_data.h"
#define AUDIO_DATA g711u_test_data
#define AUDIO_FRAME_DURATION_MS (20)
#define AUDIO_FRAME_LEN (160)
#endif

// Define this while needing dump the received audio/video data
#undef DUMP_RECEIVED
