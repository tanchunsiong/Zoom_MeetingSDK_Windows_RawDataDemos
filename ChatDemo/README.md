# Meeting SDK for Windows - Chat Demo

This sample shows how to join a meeting, register a chat event listener, and send a chat message with the Windows Meeting SDK.

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

Create `ChatDemo/config.json` with values for your environment:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample is configured to fetch the auth token from `webservice_endpoint` by default because `isJWTWebService = true` in `MSDK_ChatDemo.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `ChatDemo` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_ChatDemo.cpp`.

## What the sample does

- Authenticates with the Meeting SDK.
- Joins the configured meeting.
- Gets `IMeetingChatController` and attaches `MeetingChatEventListener`.
- Sends a demo message to everyone in the meeting from `onInMeeting()`.

## Troubleshooting

- If `json/json.h` is missing, make sure `vcpkg integrate install` completed successfully.
- If authentication fails, verify the `webservice_endpoint` you supplied returns a valid SDK auth token.
- If sending chat fails, confirm chat is allowed in the target meeting.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and address any API changes required by the newer SDK.

