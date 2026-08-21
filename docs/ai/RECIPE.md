# IoT CallKit device demo

## Overview

Run the device-side companion for an Agora IoT CallKit workflow and connect it to mobile callers.

Use this recipe when you want a working reference for device client, Agora RTC, call signaling, embedded setup.

Repository: [AgoraIO-Community/ag-iot-callkit-device-demo](https://github.com/AgoraIO-Community/ag-iot-callkit-device-demo)

## Prerequisites

- An Agora account and project App ID.
- The platform toolchain required by this repository.
- A test device, simulator, browser, or runtime that supports the sample's audio/video path.
- A second RTC client or sample instance for validating remote publish and subscribe behavior.

## Configure

Create or update the project configuration with:

- `AGORA_APP_ID`
- `AGORA_TOKEN`

For local testing, some Agora projects allow tokenless joins when App Certificate is disabled. For production, generate RTC tokens on a trusted server and renew them before expiration.

For device demos, keep credentials out of firmware committed to source control.

## Run The Sample

Build the device-side sample using the repository instructions, configure Agora credentials, then pair it with the companion client. Verify channel join, call state, and media exchange from the device endpoint.

## What To Check

- The app registers Agora RTC event handlers before joining a channel.
- Local media preview or local audio state appears after permissions are granted.
- Remote users are rendered or heard when they publish audio/video.
- `user-published` or native user-joined callbacks are handled for each media type the sample supports.
- Leaving the channel stops local capture and releases RTC resources.

## Production Notes

- Never ship an App Certificate or privileged token generator in a client app.
- Renew tokens from the SDK expiration callback before the current token expires.
- Use subscriber/audience roles for users who should not publish media.
- Keep UID types consistent across clients in the same channel.
- For browser-based samples, deploy over HTTPS except when testing on `localhost`; for native samples, validate camera, microphone, and audio routing permissions on target devices.

## Implementation Prompt

```text
You are working in `AgoraIO-Community/ag-iot-callkit-device-demo`. Add or adapt this sample so a developer can run the iot callkit device demo end to end with Agora RTC.

Goals:
- Preserve the existing sample architecture and README commands.
- Configure Agora credentials through environment variables or the platform-native configuration already used by the repo.
- Register RTC event handlers before joining the channel.
- Join a test channel, publish local media when the sample role allows it, and subscribe to remote users.
- Clean up RTC resources when leaving the channel or closing the app.
- Add production notes for token generation, renewal, and audience-only subscriber roles where applicable.

Validate by running the project-specific build or launch command from the README and testing with a second client in the same channel.
```
