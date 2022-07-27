#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "agora_iot_api.h"

#include "test_av_src.h"
#include "app_config.h"

#include "h264_test_data_352x288.h"
#include "pcm_test_data_16k_5s.h"

typedef struct pacer {
  uint32_t interval_ms;
  int64_t predict_next_time_ms;
} pacer_t;

static pacer_t* _pacer_create(uint32_t interval_ms);
static void _pacer_destroy(pacer_t *p);
static void _wait_next_pacer(pacer_t *p);

pacer_t* _pacer_create(uint32_t interval_ms)
{
  pacer_t *pacer = (pacer_t *)malloc(sizeof(pacer_t));
  if (!pacer) {
    printf("####[%s, %d] malloc failure\n", __FUNCTION__, __LINE__);
    return NULL;
  }

  pacer->interval_ms = interval_ms;
  pacer->predict_next_time_ms = 0;

  return pacer;
}

void _pacer_destroy(pacer_t *p)
{
  if (p) {
    free(p);
    p = NULL;
  }
}

void _wait_next_pacer(pacer_t *p)
{
  int64_t sleep_ms = 0;
  int64_t cur_time_ms = 0;

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  cur_time_ms = (uint64_t)ts.tv_sec * (uint64_t)1000 + ts.tv_nsec / 1000000;

  if (p->predict_next_time_ms == 0) {
    p->predict_next_time_ms = cur_time_ms + p->interval_ms;
  }

  sleep_ms = p->predict_next_time_ms - cur_time_ms;

  if (sleep_ms > 0) {
    usleep(sleep_ms * 1000);
  }

  p->predict_next_time_ms += p->interval_ms;
}

typedef struct thread_info {
    pthread_t tid;
    bool is_running;
} thread_info_t;
static thread_info_t g_thread_info_v;
static thread_info_t g_thread_info_a;

static void *_video_thread(void *arg)
{
    g_thread_info_v.is_running = true;
    void *iot_handle = arg;
    int interval_ms = 1000 / CONFIG_SEND_FRAME_RATE;
    pacer_t *pacer = _pacer_create(interval_ms);
    int frame_num = sizeof(test_video_frames)/sizeof(video_frame_t);
    int frame_index = 0;
    ago_video_frame_t v_frame = {
        .data_type      = AGO_VIDEO_DATA_TYPE_H264,
        .is_key_frame   = true,
    };
    int ret = 0;

    while (g_thread_info_v.is_running) {
        frame_index = (frame_index % frame_num);
        v_frame.video_buffer = test_video_frames[frame_index].data;
        v_frame.video_buffer_size = test_video_frames[frame_index].len;
        ret = agora_iot_push_video_frame(iot_handle, &v_frame);
        if (ret < 0) {
            printf("#### Push video frame failure: %d\n", ret);
        }

        frame_index++;
        _wait_next_pacer(pacer);
    }

    _pacer_destroy(pacer);
    return NULL;
}

static void *_audio_thread(void *arg)
{
    g_thread_info_a.is_running = true;
    void *iot_handle = arg;
    int interval_ms = (CONFIG_PCM_FRAME_LEN * 1000 / CONFIG_AUDIO_SAMPLE_RATE / CONFIG_PCM_CHANNEL_NUM / sizeof(int16_t));
    pacer_t *pacer = _pacer_create(interval_ms);
    uint32_t offset = 0;
    ago_audio_frame_t a_frame = {
        .data_type = AGO_AUDIO_DATA_TYPE_PCM
    };
    int ret = 0;

    while (g_thread_info_a.is_running) {
        a_frame.audio_buffer = (uint8_t *)pcm_test_data + offset;
        a_frame.audio_buffer_size = CONFIG_PCM_FRAME_LEN;
        ret = agora_iot_push_audio_frame(iot_handle, &a_frame);
        if (ret < 0) {
            printf("#### Push audio frame failure: %d\n", ret);
        }

        offset += CONFIG_PCM_FRAME_LEN;
        if (offset >= sizeof(pcm_test_data)) {
            /* wrap around to the beginning of audio test data */
            offset = 0;
        }
        _wait_next_pacer(pacer);
    }

    _pacer_destroy(pacer);
    return NULL;
}

int start_push_video(void *iot_handle)
{
    if (g_thread_info_v.is_running) {
        printf("#### Already starting to push video frame\n");
        return 0;
    }

    /* Create a thread to push video frame */
    int ret = pthread_create(&g_thread_info_v.tid, NULL, _video_thread, iot_handle);
    if (ret < 0) {
        printf("####[%s, line: %d] Create thread failure: %d\n", __FUNCTION__, __LINE__, ret);
        return -1;
    } else {
        printf("####[%s, line: %d] Create thread(tid: %lu) successfully\n",
                __FUNCTION__, __LINE__, g_thread_info_v.tid);
        return 0;
    }
}

int stop_push_video()
{
    printf("####[%s, line: %d]stop thread ...\n", __FUNCTION__, __LINE__);
    g_thread_info_v.is_running = false;
    int ret = pthread_join(g_thread_info_v.tid, NULL);
    if (ret < 0) {
        printf("#### and failure: %d\n", ret);
        return -1;
    } else {
        printf("#### and successfully!\n");
        g_thread_info_v.tid = 0;
        return 0;
    }
}

int start_push_audio(void *iot_handle)
{
    if (g_thread_info_a.is_running) {
        printf("#### Already starting to push audio frame\n");
        return 0;
    }

    /* Create a thread to push audio frame */
    int ret = pthread_create(&g_thread_info_a.tid, NULL, _audio_thread, iot_handle);
    if (ret < 0) {
        printf("####[%s, line: %d] Create thread failure: %d\n", __FUNCTION__, __LINE__, ret);
        return -1;
    } else {
        printf("####[%s, line: %d] Create thread(tid: %lu) successfully\n",
                __FUNCTION__, __LINE__, g_thread_info_a.tid);
        return 0;
    }
}

int stop_push_audio()
{
    printf("####[%s, line: %d] Stop thread ...\n", __FUNCTION__, __LINE__);
    g_thread_info_a.is_running = false;
    int ret = pthread_join(g_thread_info_a.tid, NULL);
    if (ret < 0) {
        printf("#### and failure: %d\n", ret);
        return -1;
    } else {
        printf("#### and successfully!\n");
        g_thread_info_a.tid = 0;
        return 0;
    }
}

// EOF