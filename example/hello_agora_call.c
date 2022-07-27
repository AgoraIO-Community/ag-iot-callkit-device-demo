#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <strings.h>
#include <signal.h>
#include <pthread.h>

#include "app_config.h"
#include "test_av_src.h"

#include "agora_iot_api.h"
#include "agora_iot_call.h"
#include "agora_iot_device_manager.h"

#define DEFAULT_ALARM_IMAGE_FILE_PATH "warning-640x640.jpg"

static agora_iot_handle_t g_iot_handle = NULL;

static void app_signal_handler(int sig)
{
  switch (sig) {
  case SIGQUIT:
  case SIGABRT:
  case SIGINT:
    // deinit agora iot
    agora_iot_deinit(g_iot_handle);
    exit(0);
    break;
  default:
    printf("#### no handler, sig %d", sig);
  }
}

static void iot_cb_call_request(const char *peer_name, const char *attach_msg)
{
  if (!peer_name) {
    return;
  }

  printf("#### Get call from peer \"%s\", attach message: %s\n", peer_name, attach_msg ? attach_msg : "null");
}

static void iot_cb_call_local_timeout(const char *peer_id)
{
  if (!peer_id) {
    return;
  }
  printf("#### Local Timeout, peer_id: %s\n", peer_id);
}

static void iot_cb_call_peer_timeout(const char *peer_id)
{
  if (!peer_id) {
    return;
  }
  printf("#### Peer Timeout, peer_id: %s\n", peer_id);
}

static void iot_cb_start_push_frame()
{
  start_push_video(g_iot_handle);
  start_push_audio(g_iot_handle);
}

static void iot_cb_stop_push_frame()
{
  stop_push_video();
  stop_push_audio();
}

static void iot_cb_call_hung_up(const char *peer_name)
{
  if (!peer_name) {
    return;
  }

  printf("#### Get hangup from peer \"%s\"\n", peer_name);
}

static void iot_cb_call_answered(const char *peer_name)
{
  printf("#### Get answer from peer \"%s\"\n", peer_name);
}

static void iot_cb_receive_audio(ago_audio_frame_t *frame)
{
  //printf("#### iot_cb_receive_audio %zu\n", frame->audio_buffer_size);

  int ret = 0;
  static FILE *fp = NULL;
  if (!fp) {
    fp = fopen("receive.bin", "wb");
  }

  ret = fwrite(frame->audio_buffer, 1, frame->audio_buffer_size, fp);
}

static void iot_cb_key_frame_request(void) {
  printf("#### Get key frame request from remote\n");
}

static void iot_cb_target_bitrate_changed(uint32_t target_bitrate) {
  printf("#### Get target bitrate: %d\n", (int)target_bitrate);
  // should update bitrate of encoder here
}

static agora_iot_audio_config_t audio_config = {
  .audio_codec_type = AGO_AUDIO_CODEC_TYPE_G722,
  .pcm_sample_rate = CONFIG_AUDIO_SAMPLE_RATE,
  .pcm_channel_num = CONFIG_PCM_CHANNEL_NUM,
};

static int _get_file_info(agora_alarm_file_info_t *file_info)
{
  char *file_name = strrchr(DEFAULT_ALARM_IMAGE_FILE_PATH, '/') + 1;
  FILE *fd = fopen(DEFAULT_ALARM_IMAGE_FILE_PATH, "rb");
  if (fd < 0) {
      printf("#### open file(%s) error.", DEFAULT_ALARM_IMAGE_FILE_PATH);
      return -1;
  }
  /* get the size of the file and read the file into the buffer */
  fseek(fd, 0, SEEK_END);
  size_t file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  char *file_buf = (char *)calloc(file_size, sizeof(char));
  if (!file_buf) {
    printf("#### no memory for file buffer");
    fclose(fd);
    return -1;
  } else {
    fread(file_buf, sizeof(char), file_size, fd);
    fclose(fd);
  }

  file_info->name = file_name;
  file_info->buf  = file_buf;
  file_info->size = file_size;
  return 0;
}

static void _help_info()
{
  printf("#### Usage: hello_agora_call [-p product_key] [-u user_id] [-d device_id]\n");
  printf("#### Use the default values as those unspecified options\n");
}

int main(int argc, char *argv[])
{
  char user_id[64] = CONFIG_USER_ID;
  char device_id[64] = CONFIG_DEVICE_ID;
  char product_key[64] = CONFIG_PRODUCT_KEY;
  for (int i = 1; i < argc; i++) {
    if (argv[i] && 0 == strcmp("-u", argv[i])) {
      if (i + 1 < argc && argv[i+1]) {
        snprintf(user_id, sizeof(user_id), "%s", argv[i+1]);
        i++;
      }
    } else if (argv[i] && 0 == strcasecmp("-d", argv[i])) {
      if (i + 1 < argc && argv[i+1]) {
        snprintf(device_id, sizeof(device_id), "%s", argv[i+1]);
        i++;
      }
    } else if (argv[i] && 0 == strcasecmp("-p", argv[i])) {
      if (i + 1 < argc && argv[i+1]) {
        snprintf(product_key, sizeof(product_key), "%s", argv[i+1]);
        i++;
      }
    } else if (argv[i] && 0 == strcasecmp("--help", argv[i])) {
      _help_info();
      return 0;
    }
  }

  int result = 0;
  char cmd[10] = { 0 };
  char peer_id[64] = { 0 };

  agora_iot_device_info_t device_info = { 0 };

  signal(SIGINT, app_signal_handler);

  /**
   * Step 0-0: Register to cloud to get domain and thing_name,
   * and bind the device to the user with the user id, then get the client id of the user.
   */
  result = agora_iot_register_and_bind(CONFIG_MASTER_SERVER_URL, product_key, device_id, NULL, NULL, &device_info);
  if (0 != result) {
    printf("#### register device into aws failure: %d\n", result);
    return -1;
  }
  char user_client_id[64] = { 0 };
  result = agora_iot_query_user(CONFIG_MASTER_SERVER_URL, product_key, device_id, user_client_id);
  if (0 != result) {
    printf("#### query the user client id failure: %d\n", result);
  }

  /**
   * @brief Step 0-1: Activate your license and get the certificate
   * To avoid activating your license and reducing your license count while running the demo every time,
   * should save the certificate into files or flash memory, and next time use the certificate.
   * Don't need to activate to get a new certificate again.
   */
  char *certificate = NULL;
  if (0 != agora_iot_license_activate(CONFIG_AGORA_APP_ID, CONFIG_CUSTOMER_KEY, CONFIG_CUSTOMER_SECRET,
                                      CONFIG_PRODUCT_KEY, device_id, &certificate)) {
    printf("#### cannot activate agora license !\n");
    goto EXIT;
  } else {
    printf("#### license certificate:\n%s\n", certificate);
  }

  // Initialize IOT SDK
  agora_iot_rtc_callback_t rtc_cb = {
      .cb_start_push_frame        = iot_cb_start_push_frame,
      .cb_stop_push_frame         = iot_cb_stop_push_frame,
      .cb_target_bitrate_changed  = iot_cb_target_bitrate_changed,
      .cb_key_frame_requested     = iot_cb_key_frame_request,
      .cb_receive_audio_frame     = iot_cb_receive_audio
  };
  agora_iot_call_callback_t call_cb = {
      .cb_call_request        = iot_cb_call_request,
      .cb_call_hung_up        = iot_cb_call_hung_up,
      .cb_call_answered       = iot_cb_call_answered,
      .cb_call_local_timeout  = iot_cb_call_local_timeout,
      .cb_call_peer_timeout   = iot_cb_call_peer_timeout
  };
  agora_iot_config_t cfg = {
      .app_id               = CONFIG_AGORA_APP_ID,
      .product_key          = product_key,
      .client_id            = device_info.client_id,
      .domain               = device_info.domain, // domain host for dp
      .root_ca              = CONFIG_AWS_ROOT_CA, // aws root ca buffer for dp
      .client_crt           = device_info.certificate, // client certificate buffer for dp
      .client_key           = device_info.private_key, //  client private key buffer for dp
      .enable_rtc           = true,
      .certificate          = certificate,
      .enable_recv_audio    = true,
      .enable_recv_video    = true,
      .enable_audio_config  = true,
      .audio_config         = audio_config,
      .rtc_cb               = rtc_cb,
      .slave_server_url     = CONFIG_SLAVE_SERVER_URL,
      .call_cb              = call_cb
  };
  g_iot_handle = agora_iot_init(&cfg);
  if (!g_iot_handle) {
    // if init failed, no need to deinit
    printf("#### agora_iot_init failed\n");
    goto EXIT;
  }

  do {
    memset(cmd, 0, sizeof(cmd));
    memset(peer_id, 0, sizeof(peer_id));

    printf("#### Input your command: \"call\", \"hangup\", \"answer\", \"alarm\", or \"quit\" \n");
    scanf(" %9s", cmd);

    if (strncasecmp(cmd, "quit", strlen("quit")) == 0) {
      printf("#### quit!!!\n");
      break;
    } else if (0 == strncasecmp(cmd, "alarm", strlen("alarm"))) {
      printf("#### Input the binding user account: ");
      scanf(" %63s", peer_id);

      agora_alarm_file_info_t file_info = {
        /* If always use the default image, you should not rename the image */
        .rename = false
      };
      if (0 != _get_file_info(&file_info)) {
        printf("#### Can not access the default image file\n");
        continue;
      }

      char extra_msg[] = "someone passes by";
      result = agora_iot_alarm(g_iot_handle, peer_id, extra_msg, AG_ALARM_TYPE_VAD, &file_info);

      free(file_info.buf);

      if (0 != result) {
        printf("#### alarm failure: %d\n", result);
        printf("#### check your input parameters and retry ...\n");
      }
    } else if (strncasecmp(cmd, "call", strlen("call")) == 0) {
      printf("#### Input the peer's ID (ps. your binding user ID: %s): ", user_client_id);
      scanf(" %63s", peer_id);

      char extra_msg[] = "someone passes by";
      result = agora_iot_call(g_iot_handle, peer_id, extra_msg);

      if (0 != result) {
        printf("#### call failure: %d\n", result);
        printf("#### check your input parameters and retry ...\n");
      }

    } else if (strncasecmp(cmd, "hangup", strlen("hangup")) == 0) {
#if 0
      g_app.b_call_session_ongoing = false;
#endif
      agora_iot_hang_up(g_iot_handle);
    } else if (strncasecmp(cmd, "answer", strlen("answer")) == 0) {
      agora_iot_answer(g_iot_handle);
    }else {
      printf("#### unknown command");
    }
  } while (1);

  // deinit agora iot
  agora_iot_deinit(g_iot_handle);

EXIT:
  // free license certificate
  if (certificate) {
    free(certificate);
    certificate = NULL;
  }
  return 0;
}