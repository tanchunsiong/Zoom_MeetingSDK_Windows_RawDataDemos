# Meeting SDK for Windows - Local Recording

This sample shows how to start standard local recording from the Windows Meeting SDK.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- Host, co-host, or local-recording permission in the target meeting

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `LocalRecording/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_LocalRecording.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `LocalRecording` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_LocalRecording.cpp`.

## What the sample does

- Joins the configured meeting.
- Waits until `CanIStartRecording()` succeeds.
- Gets the meeting recording controller.
- Calls `StartRecording()`.
- Lets Zoom save and convert the recording output under your `Documents\Zoom` folder after the meeting ends.

`StartRecording()` and `StartRawRecording()` are separate flows. Do not use them at the same time in the same session.

## Troubleshooting

- If recording does not start, verify the meeting host has granted local-recording permission.
- If `json/json.h` is missing, verify your `vcpkg` integration.
- If you need raw YUV output instead of standard local recording, use one of the raw-data samples instead of this project.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

