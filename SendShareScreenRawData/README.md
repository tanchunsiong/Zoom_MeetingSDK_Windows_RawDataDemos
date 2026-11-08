# Meeting SDK for Windows - Send Share Raw Data

This sample shows how to provide a virtual share source and push raw frames into an active meeting share.

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

Create `SendShareScreenRawData/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_SendShareScreenRawData.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `SendShareScreenRawData` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_SendShareScreenRawData.cpp`.

## What the sample does

- Joins a meeting and waits until the SDK reports the client is fully in meeting.
- Checks `CanStartShare()` before starting a virtual share session.
- Creates `ZoomSDKShareSource`, an implementation of `IZoomSDKShareSource`.
- Calls `GetRawdataShareSourceHelper()` and `setExternalShareSource(...)` to register that source.
- Streams frames from `video_source` in `ZoomSDKShareSource.cpp`.

## Troubleshooting

- If share does not start, make sure nobody else is sharing and that the meeting allows screen sharing.
- If `json/json.h` is missing, verify your `vcpkg` integration.
- If OpenCV libraries are missing, install the matching OpenCV packages for your chosen triplet.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code that changed with the SDK release.

