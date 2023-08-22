/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Agora Lab, Inc (http://www.agora.io/)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

// Demo-self header files, usually to define some values for reusing
#include "hello_doorbell_comm.h"

static FILE *g_dump_audio = NULL;
static FILE *g_dump_video = NULL;

extern app_t g_app;
// app_t g_app = {
//   .b_login_success = false,
//   .b_push_thread_run = false,
//   .b_exit = false,
//   .video_thread_id = 0,
//   .audio_thread_id = 0,
// };

typedef struct {
  uint32_t interval_ms;
  int64_t predict_next_time_ms;
} pacer_t;

static void *_pacer_create(uint32_t interval_ms)
{
  pacer_t *pacer = (pacer_t *)malloc(sizeof(pacer_t));

  pacer->interval_ms = interval_ms;
  pacer->predict_next_time_ms = 0;

  return pacer;
}

static void _pacer_destroy(void *pacer)
{
  if (pacer) {
    free(pacer);
  }
}

static void _wait_for_next_pace(void *pacer)
{
  pacer_t *pc = pacer;
  int64_t sleep_ms = 0;
  int64_t cur_time_ms = 0;

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  cur_time_ms = (uint64_t)ts.tv_sec * (uint64_t)1000 + ts.tv_nsec / 1000000;

  if (pc->predict_next_time_ms == 0) {
    pc->predict_next_time_ms = cur_time_ms + pc->interval_ms;
  }

  sleep_ms = pc->predict_next_time_ms - cur_time_ms;

  if (sleep_ms > 0) {
    usleep(sleep_ms * 1000);
  }

  pc->predict_next_time_ms += pc->interval_ms;
}

static void signal_handler(int sig)
{
  switch (sig) {
  case SIGQUIT:
  case SIGABRT:
  case SIGINT:
    g_app.b_exit = true;
    if (g_app.b_push_thread_run) {
      printf("Hang up the call.\n");
      g_app.b_push_thread_run = false;
      agora_iot_hang_up();
    }
    if (g_dump_audio) {
      fclose(g_dump_audio);
      g_dump_audio = NULL;
    }
    if (g_dump_video) {
      fclose(g_dump_video);
      g_dump_video = NULL;
    }
    break;
  default:
    printf("no handler, sig %d", sig);
  }
}

static int send_video_frame(uint8_t *data, uint32_t len)
{
  int rval;

  // API: send video data
  agora_video_frame_t ago_frame = { 0 };
  ago_frame.data_type = SEND_VIDEO_DATA_TYPE;
  ago_frame.is_key_frame = true;
  ago_frame.video_buffer = data;
  ago_frame.video_buffer_size = len;
  ago_frame.fps = CONFIG_SEND_FRAME_RATE;

  rval = agora_iot_push_video_frame(&ago_frame);
  if (rval < 0) {
    printf("Failed to push video frame\n");
    return -1;
  }
  return 0;
}

static int send_audio_frame(uint8_t *data, uint32_t len)
{
  int rval;

  // API: send audio data
  agora_audio_frame_t ago_frame = { 0 };
  ago_frame.data_type = SEND_AUDIO_DATA_TYPE;
  ago_frame.audio_buffer = data;
  ago_frame.audio_buffer_size = len;
  rval = agora_iot_push_audio_frame(&ago_frame);
  if (rval < 0) {
    printf("Failed to push audio frame\n");
    return -1;
  }
  return 0;
}

static void *video_send_thread(void *threadid)
{
  int video_send_interval_ms = 1000 / CONFIG_SEND_FRAME_RATE;
  void *pacer = _pacer_create(video_send_interval_ms);
  int num_frames = sizeof(TEST_VIDEO_FRAMES) / sizeof(TEST_VIDEO_FRAMES[0]);
  int i = 0;

  while (g_app.b_push_thread_run) {
    i = (i % num_frames); // calculate frame indexS
    send_video_frame(TEST_VIDEO_FRAMES[i].data, TEST_VIDEO_FRAMES[i].len);
    i++;
    // sleep and wait until time is up for next send
    _wait_for_next_pace(pacer);
  }
  _pacer_destroy(pacer);
  return NULL;
}

static void *audio_send_thread(void *threadid)
{
  int audio_send_interval_ms = AUDIO_FRAME_DURATION_MS;
  void *pacer = _pacer_create(audio_send_interval_ms);
  uint32_t offset = 0;

  while (g_app.b_push_thread_run) {
    send_audio_frame((uint8_t *)AUDIO_DATA + offset, AUDIO_FRAME_LEN);
    offset += AUDIO_FRAME_LEN;
    /* wrap around to the beginning of audio test data */
    if ((offset + AUDIO_FRAME_LEN) >= sizeof(AUDIO_DATA)) {
      offset = 0;
    }
    // sleep and wait until time is up for next send
    _wait_for_next_pace(pacer);
  }
  _pacer_destroy(pacer);
  return NULL;
}

void install_signal_handler(void)
{
  signal(SIGINT, signal_handler);
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = signal_handler;
  sa.sa_flags = 0; // not SA_RESTART!;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}

void iot_cb_call_request(const char *peer_name, const char *attach_msg)
{
  if (!peer_name) {
    return;
  }

  printf("Get call from peer \"%s\"\n", peer_name);

  // auto answer, remove it if not needed
  agora_iot_answer();
}

void iot_cb_start_push_frame(void)
{
  int rval;
  // printf("Ready to push frames type: %d\n", push_type);

  // // Note: you may start video encoder here

  // // record push type
  // g_push_type |= push_type;

  if (g_app.b_push_thread_run) {
    printf("Already pushing frames!\n");
    return;
  }

  g_app.b_push_thread_run = true;
  rval = pthread_create(&g_app.video_thread_id, NULL, video_send_thread, 0);
  if (rval < 0) {
    printf("Unable to create video push thread\n");
    return;
  }

  rval = pthread_create(&g_app.audio_thread_id, NULL, audio_send_thread, 0);
  if (rval < 0) {
    printf("Unable to create audio push thread\n");
    return;
  }
}

void iot_cb_stop_push_frame(void)
{
  printf("Stop pushing frames\n");

  // // Note: you may stop video encoder here

  // // record push type
  // g_push_type &= ~push_type;

  // if (g_push_type == 0x00) {
    g_app.b_push_thread_run = false;
    if (g_app.video_thread_id) {
      pthread_join(g_app.video_thread_id, NULL);
      g_app.video_thread_id = 0;
    }
    if (g_app.audio_thread_id) {
      pthread_join(g_app.audio_thread_id, NULL);
      g_app.audio_thread_id = 0;
    }
  // }
#ifdef CONFIG_FILE_PLAYER_TEST_ENABLED
  if (g_file_player) {
    agora_iot_file_player_stop(g_file_player);
    g_file_player = NULL;
  }
#endif
}

#define DUMP_RECEIVED
void iot_cb_receive_audio_frame(agora_audio_frame_t *frame)
{
#ifdef DUMP_RECEIVED
  printf("Receive audio frame, size %d\n", frame->audio_buffer_size);

  // Note: you may send the frame to audio player here
  int ret = 0;
  if (!g_dump_audio) {
    g_dump_audio = fopen("receive_audio.bin", "wb");
  }

  ret = fwrite(frame->audio_buffer, 1, frame->audio_buffer_size, g_dump_audio);
#endif
}

void iot_cb_receive_video_frame(agora_video_frame_t *frame)
{
#ifdef DUMP_RECEIVED
  printf("Receive video frame, size %d\n", frame->video_buffer_size);

  // Note: you may send the frame to video decoder here
  int ret = 0;
  if (!g_dump_video) {
    g_dump_video = fopen("receive_video.bin", "wb");
  }

  ret = fwrite(frame->video_buffer, 1, frame->video_buffer_size, g_dump_video);
#endif
}

void iot_cb_key_frame_requested(void)
{
  printf("Please notify the encoder to generate key frame immediately\n");
  // Note: you should force IDR frame in case of H264 encoder
}

void iot_cb_target_bitrate_changed(uint32_t target_bitrate)
{
  printf("Bandwidth change detected. Please adjust encoder bitrate to %u kbps\n", target_bitrate / 1000);
  // Note: you should update target bitrate setting in case of H264 encoder
}

// Just for test

void iot_cb_connect_status(uint32_t status) {
  printf("Agora server connection status change: %d\n", (int)status);
}