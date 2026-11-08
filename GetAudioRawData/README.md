# Meeting SDK for Windows - Get Audio Raw Data

This sample shows how to start raw recording and subscribe to mixed and one-way audio callbacks from the Meeting SDK.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- Host, co-host, or raw-recording permission in the target meeting
- The client must join computer audio

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `GetAudioRawData/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_GetAudioRawData.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `GetAudioRawData` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_GetAudioRawData.cpp`.

## What the sample does

- Joins the configured meeting.
- Waits until `CanIStartRecording()` succeeds.
- Calls `StartRawRecording()`.
- Gets `IZoomSDKAudioRawDataHelper` and subscribes `ZoomSDKAudioRawDataDelegate`.
- Receives audio frames in `onMixedAudioRawDataReceived` and `onOneWayAudioRawDataReceived`.

The sample implementation writes PCM data that you can convert later with tools such as `ffmpeg`.

## Troubleshooting

- If `StartRawRecording()` fails, verify raw-recording permission for the meeting.
- If you hit `SDKERR_NOT_JOIN_AUDIO`, make sure the client joined computer audio.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

