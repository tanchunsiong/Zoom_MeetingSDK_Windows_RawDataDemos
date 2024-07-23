
# Meeting SDK for Windows - Get Audio Raw Demo

A Windows C++ Application demonstrate Zoom Meeting SDK Getting Raw Audio features

## Additional dependency libs.

This sample requires CURL and Json Parser

We are using vcpkg to install these dependencies on Windows

### Installing vcpkg
You might need to use Powershell (as administrator) or Windows Terminal to execute the sh script files
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh 
./vcpkg integrate install --vcpkg-root c:\vcpkg
```

### Installing cURL and JSON Parser using vckpg

`./vcpkg install curl`

`./vcpkg install jsoncpp`

## Add a configuration file named `config.json`

This is file which is used to pass in your sdk auth signature (aka jwt token, aka auth signature)

```
{
  "sdk_jwt": "<your_sdk_jwt>",
  "meeting_number": <meeting_number_to_join>,
  "passcode": "<passcode>",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4",
  "zak":""
}
```

The app will try to join the meeting follow the Meeting Number you specified in the config.json. 

## Add the sdk files into a folder name `SDK`

The folder should look something like this

- SDK
	- x64
	- x86
	- CHANGELOG.MD
	- OSS-LICENSE.txt
	- README.md
	- version.txt



## Open and Run Project

Right click on "GetAudioRawData" and "Startup as Project" in Visual Studio


Hit F5 or click from menu "Debug" -> "Start Debugging" in x86 or x64 to launch the application.


## Error

if you are getting an error about not being able to open source json/json.h , include this in your

Visual Studio Project -> Properties. Under C/C++ ->General ->Additional Include Directories,


## Getting Started

The main method, or main entry point of this application is at `MSDK_xxxxxxxDemo.cpp`

From a high level point of view it will do the below.
- Join a meeting
- Wait for callback or status update. There are some prerequistes before you can get audio raw data. The `CanIStartRecording()` method helps to check if you have fulfilled these requirements
  - You need to have host, co-host or recording permissions
  - You need to be in-meeting. This is the status when you have fully joined a meeting.
- Get the Meeting Recording Controller
  - Use the Meeting Recording Controller to call `StartRawRecording()`. Do note that you can only either run `StartRecording()` or `StartRawRecording()`. You cannot run them both at once.
- Thereafter, you should be able to `GetAudioRawdataHelper()`, which is used to subscribe to your `ZoomSDKAudioRawDataDelegate.cpp` (An implementation of `IZoomSDKAudioRawDataDelegate`). If you encounter error calling `GetAudioRawdataHelper(), you might be calling it without the prequisites. 
  - In the implementation(`ZoomSDKAudioRawDataDelegate.cpp`), `onMixedAudioRawDataReceived` and `onOneWayAudioRawDataReceived` will start to receive callbacks.
  - I'm using `onMixedAudioRawDataReceived` to save the PCM buffer into a file. You will need to use a converter such as ffmpeg to convert this PCM file into a playable wav or mp3 file.


# Upgrading Guide

You will need to download the latest Meeting SDK Windows for c++ from marketplace.zoom.us

Replace the files in the folder `SDK` with those found in the downloaded files from marketplace.zoom.us

You will need to ensure any missing abstract classes are implemented etc... before you can compile and upgrade to a newer SDK version.

# Troubleshooting Guide

## Error

if you are getting an error about not being able to open source json/json.h , include this in your

Visual Studio Project -> Properties. Under C/C++ ->General ->Additional Include Directories,

 ### x64
 C:\yourpath\whereyouinstalled\vcpkg\packages\jsoncpp_x64-windows\include
 
 or

 ### x86
 C:\yourpath\whereyouinstalled\vcpkg\packages\jsoncpp_x86-windows\include
 
 ## SDK_ERR_NOT_JOIN_AUDIO

you will need to ensure when joining a meeting this is set to false 
	`joinMeetingWithoutLoginParam.isAudioOff = false;`

here's an additional parameter to enable auto joining of audio

```
//set join audio to true
ZOOM_SDK_NAMESPACE::IAudioSettingContext* pAudioContext = settingService->GetAudioSettings();
	if (pAudioContext)
	{	
		pAudioContext->EnableAutoJoinAudio(true);
	}
```


