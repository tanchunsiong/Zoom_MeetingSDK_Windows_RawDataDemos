# Meeting SDK for Windows - Breakout Demo

This sample shows the basic wiring for the Meeting SDK breakout-room controller APIs.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- Breakout rooms enabled for the account and meeting you are testing
- Host or co-host permissions for breakout-room management

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `BreakoutDemo/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_BreakoutDemo.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `BreakoutDemo` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_BreakoutDemo.cpp`.

## What the sample does

- Authenticates with the Meeting SDK.
- Starts a meeting by default because `isStartMeeting = true` in the current code.
- Attaches `IMeetingBOControllerEventListener`.
- Accesses breakout helpers such as `GetBOAdminHelper()`, `GetBOCreatorHelper()`, and `GetBODataHelper()`.
- Enumerates breakout meetings and unassigned users once the client is in meeting.

## Notes

- This sample is still a starter project rather than a complete breakout workflow.
- If you want to join an existing meeting instead of starting one, review the `isStartMeeting` branch in `MSDK_BreakoutDemo.cpp`.
- Depending on your account setup, you may need additional start-meeting parameters for your host flow.

## Troubleshooting

- If breakout helpers return empty data, verify breakout rooms are enabled and you are testing with host-level permissions.
- If authentication fails, confirm `sdk_jwt` matches the Meeting SDK app you are using.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.


