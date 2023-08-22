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
#include <malloc.h>
#include <termio.h>
#include <sys/time.h>
#include <stdlib.h>

#include "agora_iot_api.h"
#include "agora_iot_device_manager.h"

#include "app_config.h"
#include "hello_doorbell_comm.h"

app_t g_app = {
  .b_exit = false,
};

static char g_node_id[64] = { 0 };
static char g_node_secret[64] = { 0 };
static char g_user_id[64] = { 0 };

int main(int argc, char *argv[])
{
  int ret = 0;
  if (argc == 4) {
    strncpy(g_node_id, argv[1], sizeof(g_node_id));
    strncpy(g_node_secret, argv[2], sizeof(g_node_secret));
    strncpy(g_user_id, argv[3], sizeof(g_user_id));
  } else {
    printf("\n----------------------------------------------\n");
    printf("    Please input your node ID and node secret as this:\n");
    printf("    ./hello_doorbell_2 node_id node_secret bind_user_id\n");
    printf("----------------------------------------------\n\n");
    return -1;
  }

  install_signal_handler();

  // 1. activate deivce
  /* Once device activated, you can just write activate_res to local device */
  /* And it will speed up the bring up of device */
  char activate_res[1024] = { 0 };
  if (0 != agora_iot_node_activate(CONFIG_SERVER_URL, CONFIG_AGORA_APP_ID, g_node_id, g_node_secret, activate_res,
                                   sizeof(activate_res))) {
    printf("cannot activate agora license !\n");
    goto EXIT;
  }

  // 2. connect to agora iot service
  agora_iot_config_t cfg = {
    .app_id                     = CONFIG_AGORA_APP_ID,
    .activate_res               = activate_res,
    .event_handler = {
      .on_start_push_frame        = iot_cb_start_push_frame,
      .on_stop_push_frame         = iot_cb_stop_push_frame,
      .on_receive_audio_frame     = iot_cb_receive_audio_frame,
      .on_receive_video_frame     = iot_cb_receive_video_frame,
      .on_target_bitrate_changed  = iot_cb_target_bitrate_changed,
      .on_key_frame_requested     = iot_cb_key_frame_requested,
      .on_call_request            = iot_cb_call_request,
      .on_connect_status          = iot_cb_connect_status,
    },
    .audio_codec                  = INTERNAL_AUDIO_ENC_TYPE,
    .max_possible_bitrate         = DEFAULT_MAX_BITRATE,
    .min_possible_bitrate         = DEFAULT_MIN_BITRATE,
    .area_code                    = AGORA_AREA_CODE_GLOB,
    .log_cfg = {
      .disable_log                = false,
      .log_level                  = AGORA_LOG_DEBUG,
    }
  };
  ret = agora_iot_init(&cfg);
  if (0 != ret) {
    printf("agora_iot_init failed\n");
    goto EXIT;
  }

  // 3. Action: call, anser or hangup
  while (!g_app.b_exit) {
    switch (getchar()) {
    case 'c': // call
      if (0 != agora_iot_call(g_user_id, "This is a call test")) {
        printf("------- call failed.\n");
      }
      // TODO: should start ring at here until recive answer/hangup/timeout callback
      break;
    case 'a': // answer
      agora_iot_answer();
      break;
    case 'h': // hangup
      agora_iot_hang_up();
      break;
    case 'q': // quit
      g_app.b_exit = true;
      break;
    default:
      break;
    }
  }

  iot_cb_stop_push_frame();
  agora_iot_fini();
EXIT:
  return 0;
}
