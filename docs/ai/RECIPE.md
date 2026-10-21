# IoT CallKit Device Demo

## Overview

Run the device-side companion for an Agora IoT CallKit workflow and connect it to mobile callers.

Repository: [AgoraIO-Community/ag-iot-callkit-device-demo](https://github.com/AgoraIO-Community/ag-iot-callkit-device-demo)

## Prerequisites and Setup

1. Provision the Agora Link service and a device activation pair (node_id/node_secret), following the repository wiki and README.
2. Set the project App ID in `example/inc/app_config.h`. Keep the device activation secret local.
3. Run `make` inside `example/` with the SDK's supported toolchain and libraries. Use a networked Ubuntu 18.04+ target as described in the README.
4. Run the documented test command `./hello_link device_node_id device_node_secret app_node_id`, substituting your activation pair and target app node ID. Run from `example/` with `LD_LIBRARY_PATH=../lib` so the loader can find `libagora-iot-sdk.so`; the README's `./bin/test` name differs from the Makefile's `hello_link` output.

## What To Check

- Use `c` to call the target app, `h` to hang up, `a` to answer and `q` to exit.
- Confirm the mobile companion receives the intended call and two-way media works.
- Test incoming rejection and a second call after teardown.

## Integration Notes

This uses device activation and Agora Link call signaling, not a generic RTC channel-name configuration. Confirm license and SDK availability before building.

Keep App Certificates, customer secrets and privileged token generation on a trusted backend for production. Use only your own test credentials while configuring historical demos; do not commit them. Preserve the sample's existing dependency versions and lifecycle until a separate migration is tested.

## Source References

- [README.md](../../README.md)
- [example/inc/app_config.h](../../example/inc/app_config.h)

## Validation Status

This recipe was checked against the repository's source and setup documents. The platform build and live service/device flow have not been run as part of this documentation change. Complete the checks above in an environment with the required SDKs, hardware and service access before treating it as a validated deployment.

## Implementation Prompt

```text
You are working in AgoraIO-Community/ag-iot-callkit-device-demo. Use docs/ai/RECIPE.md and its linked source files to adapt the IoT CallKit Device Demo reference to this project.

Start by inspecting the actual sample directory and configuration described above. Resolve its listed missing prerequisites before claiming it is runnable. Preserve the existing architecture and compatible dependency versions. Implement the recipe's feature-specific flow, permissions and resource cleanup. Use a trusted backend for privileged credentials and tokens in production.

Run the repository-specific build or launch flow, then perform each What To Check item. Report the commands run, the observed results and anything blocked by service access, hardware or incomplete sample code. Do not claim live validation from a documentation or catalog build alone.
```
