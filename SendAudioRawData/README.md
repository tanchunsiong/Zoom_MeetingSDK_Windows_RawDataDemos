# Meeting SDK for Windows - Send Audio Raw Data

This sample shows how to register a virtual microphone and send PCM audio into a meeting.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- The meeting client must join computer audio before sending

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `SendAudioRawData/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample is configured to fetch the auth token from `webservice_endpoint` by default because `isJWTWebService = true` in `MSDK_SendAudioRawData.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `SendAudioRawData` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_SendAudioRawData.cpp`.

## What the sample does

- Joins the meeting.
- Gets `IZoomSDKAudioRawDataHelper` with `GetAudioRawdataHelper()`.
- Registers `ZoomSDKVirtualAudioMicEvent` with `setExternalAudioSource(...)`.
- Starts sending PCM data from `onMicStartSend()`.

The sample code uses `PlayAudioFileToVirtualMic` to feed PCM audio to the meeting.

## Audio notes

- Use PCM audio that matches the expectations of the SDK path you are testing.
- The sample comments and README examples assume mono, 16-bit PCM.

## Troubleshooting

- If you hit `SDKERR_NOT_JOIN_AUDIO`, make sure the client joined computer audio.
- The sample also enables auto-join audio through `EnableAutoJoinAudio(true)`.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

