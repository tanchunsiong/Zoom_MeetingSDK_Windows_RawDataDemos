# WPF Zoom Meeting SDK Demo

This sample wraps the native Zoom Meeting SDK in a C++/CLI layer and exposes it to a WPF desktop app.

## Project layout

```text
WPFSkeletonDemo/
  ZoomSDKWrapper/                 C++/CLI wrapper project
  WPFSkeletonDemo/                WPF application project
  WPFSkeletonDemo.sln             Visual Studio solution
  README.md
```

## Prerequisites

- Windows 10 or 11
- Visual Studio 2022 with:
  - .NET desktop development
  - Desktop development with C++
- The Zoom Meeting SDK unpacked to the repository-root `SDK/` folder
- x64 build target only

The wrapper project references:

- `..\..\SDK\x64\h`
- `..\..\SDK\x64\lib\sdk.lib`
- `..\..\SDK\x64\bin\*.dll`

## Configuration

The WPF app expects `WPFSkeletonDemo/WPFSkeletonDemo/config.json`, but the repository ships `config.json.example`.

Create the real config file by copying the example:

```powershell
Copy-Item .\WPFSkeletonDemo\WPFSkeletonDemo\config.json.example .\WPFSkeletonDemo\WPFSkeletonDemo\config.json
```

Then edit it:

```json
{
  "SdkJwt": "<sdk_auth_token>",
  "MeetingNumber": "1234567890",
  "Passcode": "123456",
  "UserName": "Your Display Name"
}
```

## Build

Use `MSBuild` rather than `dotnet build` because the solution mixes C++/CLI and C# projects.

```powershell
msbuild .\WPFSkeletonDemo\WPFSkeletonDemo.sln /p:Configuration=Debug /p:Platform=x64
```

You can also open `WPFSkeletonDemo.sln` in Visual Studio, select `Debug|x64`, and build there.

## Run

- Open `WPFSkeletonDemo/WPFSkeletonDemo.sln`.
- Set `WPFSkeletonDemo` as the startup project.
- Select `Debug|x64` or `Release|x64`.
- Press `F5`.

The app loads `config.json`, initializes the SDK, authenticates, and lets you join or start meetings from the UI.

## What the sample includes

- A C++/CLI wrapper project named `ZoomSDKWrapper`
- A managed `ZoomSDKManager` surface for the WPF app
- Buttons for initialize, authenticate, join, start, leave, and end meeting
- Audio and video mute controls
- Participant count and status display
- On-screen logging for common SDK events and errors

## Troubleshooting

- If the SDK cannot initialize, verify the repository-root `SDK/x64` folder is populated.
- If `ZoomSDKWrapper.dll` or Zoom SDK DLLs are missing at runtime, rebuild the solution and confirm the post-build copy steps succeeded.
- If Visual Studio complains about configuration mappings, switch the solution to `x64`.
- If authentication fails, check the `SdkJwt` value in `config.json`.

## License

This sample follows the same license as the parent Zoom Meeting SDK project.
