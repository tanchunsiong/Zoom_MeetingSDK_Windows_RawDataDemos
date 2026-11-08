# Meeting SDK for Windows - Get Share Raw Data

This sample shows how to start raw recording, create a renderer, and subscribe to a participant's share stream.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- Host, co-host, or raw-recording permission in the target meeting

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `GetShareRawData/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_GetShareRawData.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `GetShareRawData` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_GetShareRawData.cpp`.

## What the sample does

- Joins the configured meeting.
- Waits until `CanIStartRecording()` succeeds.
- Calls `StartRawRecording()`.
- Creates a renderer with `createRenderer(...)`.
- Subscribes that renderer to a user's share stream with `subscribe(userID, RAW_DATA_TYPE_SHARE)`.
- Receives YUV420 frames in `ZoomSDKRendererDelegate.cpp`.

The sample saves raw frames for inspection. If you convert them later, keep resolutions separated or normalize them first.

## Troubleshooting

- If `subscribe()` fails, verify the meeting is fully connected and raw recording has started.
- If you see corrupted output, make sure you are not mixing frames from different resolutions into the same file.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

