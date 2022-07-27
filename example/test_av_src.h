/**
 * @file file_pacer.h
 * @author sunxianbin@agora.io
 * @brief Generate a frame of the audio or the video and feed to the aPaaS SDK.
 *        Instead of capturing from the real input devcies.
 * @version 0.1
 * @date 2022-07-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __TEST_AV_SRC_H__
#define __TEST_AV_SRC_H__

int start_push_video(void *iot_handle);
int stop_push_video();
int start_push_audio(void *iot_handle);
int stop_push_audio();

#endif // __TEST_AV_SRC_H__