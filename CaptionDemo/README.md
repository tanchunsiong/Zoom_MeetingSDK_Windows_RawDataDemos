# Meeting SDK for Windows - Caption Demo

This sample shows how to work with live captions and live transcription features from the Windows Meeting SDK.

## Prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows unpacked to `SDK/` at the repository root
- `vcpkg` with `curl` and `jsoncpp`
- A `config.json` file in this folder
- Captions and transcription enabled for the account and meeting you are testing
- Appropriate meeting privileges for the caption actions you want to invoke

### Install dependencies

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

## config.json

Create `CaptionDemo/config.json`:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

This sample uses `sdk_jwt` directly by default because `isJWTWebService = false` in `MSDK_CaptionDemo.cpp`.

## Build and run

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set `CaptionDemo` as the startup project.
- Select `x64`.
- Press `F5`.

## Entry point

The sample starts in `MSDK_CaptionDemo.cpp`.

## What the sample does

- Joins the configured meeting.
- Gets `IClosedCaptionController` from the meeting service.
- Attaches `ClosedCaptionControllerEventListener`.
- Demonstrates manual caption operations such as `EnableMeetingManualCaption`, `AssignCCPrivilege`, and `SendClosedCaption`.
- Demonstrates live transcription operations such as `StartLiveTranscription`, `SetTranslationLanguage`, and `EnableReceiveSpokenLanguageContent`.

## Troubleshooting

- If callbacks never fire, verify captions or transcription are enabled for the meeting and account.
- Some caption actions require host, co-host, or explicitly assigned caption privilege.
- If `json/json.h` is missing, verify your `vcpkg` integration.

## Upgrading the SDK

- Download the latest Windows Meeting SDK.
- Replace the contents of the repository-root `SDK/` folder.
- Rebuild the solution and update any code impacted by SDK API changes.

