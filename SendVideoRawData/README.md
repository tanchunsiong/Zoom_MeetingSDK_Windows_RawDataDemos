# Meeting SDK for Windows - Send Video Raw Data

This sample shows how to expose a virtual camera to the Meeting SDK and push raw video frames into a meeting.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl`, `jsoncpp`, and `opencv`
- A `config.json` file in this folder

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp opencv
```

## config.json

Create `SendVideoRawData/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_SendVideoRawData.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `SendVideoRawData` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_SendVideoRawData.cpp`.

## What the sample does

- Joins a meeting and waits until it is fully connected.
- Gets `IZoomSDKVideoSourceHelper` from `GetRawdataVideoSourceHelper()`.
- Registers `ZoomSDKVideoSource` with `setExternalVideoSource(...)`.
- Reads frames from `video_source` and sends them with `sendVideoFrame(...)`.

`HasRawdataLicense()` is referenced in comments only. Raw data access no longer requires that older license check.

## Troubleshooting

- If the virtual camera never starts, confirm the client is fully in meeting before sending frames.
- If OpenCV errors mention missing libraries, install the matching OpenCV packages for your build triplet.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

