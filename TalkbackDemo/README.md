# Meeting SDK for Windows - Talkback Demo

This sample shows how to use the Zoom Meeting SDK talkback APIs added in the 7.0 SDK line. It authenticates into a meeting, checks talkback support, creates talkback channels, optionally invites users, adjusts background volume, and streams demo PCM audio into the first created channel.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows 7.0.2 or newer at `C:\temp\zoom-sdk-windows-7.0.2.34512\zoom-sdk-windows-7.0.2.34512`
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- A meeting/account that supports talkback
- Host or equivalent permissions to create talkback channels and invite users

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `TalkbackDemo/config.json` from `config-example.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "user_name": "Talkback Demo",
  "use_webservice_for_auth": false,
  "webservice_endpoint": "https://your-auth-endpoint.example.com",
  "start_meeting": false,
  "channel_count": 1,
  "background_volume": 0.3,
  "invite_user_ids": [12345678, 23456789],
  "auto_send_demo_audio": true,
  "sample_rate": 48000,
  "audio_channel": "mono",
  "tone_frequency_hz": 660,
  "tone_duration_seconds": 5
}
```

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `TalkbackDemo` as the startup project.
- Select `x64`.
- Press `F5`.

## What the sample does

- Authenticates with the Meeting SDK.
- Joins or starts a meeting.
- Gets `IMeetingTalkbackController` and checks `IsMeetingSupportTalkBack()`.
- Registers `IMeetingTalkbackCtrlEvent`.
- Calls `CreateChannel(channel_count)`.
- Applies `SetChannelBackgroundVolume(...)` to the first created channel.
- Batch-invites the configured `invite_user_ids`.
- Streams a generated 16-bit PCM sine wave on a worker thread to demonstrate `SendAudioDataToChannel(...)`.
- Logs participant IDs and channel user lists so you can verify invite targets.

## Troubleshooting

- If the build cannot find talkback symbols, verify `ZoomSdkDir` in `MSDK_TalkbackDemo.vcxproj` still points at the 7.0.2 SDK path.
- If `IsMeetingSupportTalkBack()` returns `false`, verify the meeting and account you are testing actually have talkback enabled.
- If invites fail, confirm the `invite_user_ids` match current in-meeting participant IDs and that your role has permission.
- If authentication fails, verify `sdk_jwt` or your `webservice_endpoint` returns a valid Meeting SDK auth token.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Update the `ZoomSdkDir` property in `MSDK_TalkbackDemo.vcxproj`.
- Rebuild and adjust any API calls that changed in later SDK releases.
