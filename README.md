# Zoom Meeting SDK for Windows Demos

This repository contains a set of Windows Meeting SDK samples for:

- receiving raw audio, video, and share data
- sending audio, video, and virtual share content
- local recording
- meeting chat, captions, and breakout-room APIs
- a WPF wrapper sample

## Clone the repository

```powershell
git clone https://github.com/tanchunsiong/Zoom_MeetingSDK_Windows_RawDataDemos.git
```

## Solutions

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln` for the native C++ samples.
- Open `WPFSkeletonDemo/WPFSkeletonDemo.sln` for the WPF sample.

## Common prerequisites

- Visual Studio 2022
- Zoom Meeting SDK for Windows
- `vcpkg` with `curl` and `jsoncpp` installed
- `opencv` installed for `SendVideoRawData` and `SendShareScreenRawData`

### Install shared C++ dependencies with vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install curl jsoncpp
```

For the OpenCV-based samples, also run:

```powershell
.\vcpkg install opencv
```

If you use explicit triplets, install packages for the same platform you plan to build, such as `x64-windows`.

## Place the Zoom SDK at the repository root

The native C++ projects reference the SDK from `$(SolutionDir)SDK\...`, so use a single shared `SDK` folder at the repository root.

Expected layout:

```text
SDK/
  x64/
  CHANGELOG.md
  OSS-LICENSE.pdf
  README.md
  version.txt
```

Download the SDK from the [Zoom Meeting SDK for Windows documentation](https://developers.zoom.us/docs/meeting-sdk/windows/).

## C++ sample configuration

Create a `config.json` file inside each C++ sample folder. A shared example that works with the current loaders is:

```json
{
  "sdk_jwt": "<sdk_auth_token>",
  "meeting_number": "1234567890",
  "passcode": "123456",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "webservice_endpoint": "https://your-auth-endpoint.example.com"
}
```

Notes:

- The code currently reads both `sdk_jwt` and `webservice_endpoint`. Each sample README calls out which auth path is enabled by default.
- The shared config loaders also read `video_source`, even in samples that do not actively stream video.

## Running a C++ sample

- Open `Zoom_MeetingSDK_Windows_RawDataDemos.sln`.
- Set the sample project you want to run as the startup project.
- Select `x64`.
- Press `F5`.

## Included samples

- `GetAudioRawData`
- `GetVideoRawData`
- `GetShareRawData`
- `SendAudioRawData`
- `SendVideoRawData`
- `SendShareScreenRawData`
- `LocalRecording`
- `ChatDemo`
- `CaptionDemo`
- `BreakoutDemo`
- `SkeletonDemo`
- `WPFSkeletonDemo`

