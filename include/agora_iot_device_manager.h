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

#ifndef __AGORA_IOT_DEVICE_MANAGER_H__
#define __AGORA_IOT_DEVICE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AGORA_IOT_CLIENT_ID_MAX_LEN 64
#define AGORA_IOT_ACTIVATE_RES_MAX_LEN 1024

/**
 * @brief Activate the device with the Agora IoT cloud.
 * 
 * @param url The URL of the Agora IoT cloud.
 * @param app_id The App ID of the Agora.
 * @param node_id The node ID of the device.
 * @param node_secret The node secret of the device.
 * @param response The response from the Agora IoT cloud.
 * @param response_len The length of the response.
 * 
 */
int agora_iot_node_activate(const char *url, const char *app_id, const char *node_id, const char *node_secret,
                            char *response, int response_len);

#ifdef __cplusplus
}
#endif

#endif /* __AGORA_IOT_DEVICE_MANAGER_H__ */