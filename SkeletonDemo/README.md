# Meeting SDK for Windows - Skeleton Demo

This is the smallest native sample in the repository. It is a good starting point for authentication, meeting join/start flow, and event wiring.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `SkeletonDemo/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample is configured to fetch the auth token from `webservice_endpoint` by default because `isJWTWebService = true` in `MSDK_SkeletonDemo.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `SkeletonDemo` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_SkeletonDemo.cpp`.

## What the sample does

- Loads meeting settings from `config.json`.
- Authenticates with the Meeting SDK.
- Joins the configured meeting by default.
- Wires the auth, network, and meeting event listeners used throughout the other native samples.

Use this project as the base when you want to try a Meeting SDK feature without the extra raw-data logic from the other demos.

## Troubleshooting

- If authentication fails, verify the `webservice_endpoint` returns a valid SDK auth token.
- If `json/json.h` is missing, verify your `vcpkg` integration.
- If you want to start meetings instead of joining, review the `isStartMeeting` flow in `MSDK_SkeletonDemo.cpp`.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

