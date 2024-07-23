
# Meeting SDK for Windows - Get Share Raw Data

A Windows C++ Application demonstrate Zoom Meeting SDK receiving Share raw data from a Zoom Meeting.

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

Right click on "GetShareRawData" and "Startup as Project" in Visual Studio


Hit F5 or click from menu "Debug" -> "Start Debugging" in x86 or x64 to launch the application.


## Error

if you are getting an error about not being able to open source json/json.h , include this in your

Visual Studio Project -> Properties. Under C/C++ ->General ->Additional Include Directories,


## Getting Started

The main method, or main entry point of this application is at `MSDK_xxxxxxxDemo.cpp`

From a high level point of view it will do the below.
- Join a meeting
- Wait for callback or status update. There are some prerequistes before you can get video raw data. The `CanIStartRecording()` method helps to check if you have fulfilled these requirements
  - You need to have host, co-host or recording permissions
  - You need to be in-meeting. This is the status when you have fully joined a meeting.
- Get the Meeting Recording Controller
  - Use the Meeting Recording Controller to call `StartRawRecording()`. Do note that you can only either run `StartRecording()` or `StartRawRecording()`. You cannot run them both at once.
- Thereafter, you should be able to `createRenderer()`, which we will later use to subscribe a specific user's share stream. You will need to put in a videoHelper (implementation of IZoomSDKRenderer) and videoSource (implementation of IZoomSDKRendererDelegate). Thereafter use your videoHelper to subscribe to a user by their userID. If you encounter error calling `subscribe()`, you might be calling it without the prequisites. Note that there is a limit of number of users you can subscribe to. This is dependent on the resolution subscribed. //to be completed
  - In the implementation(`ZoomSDKRendererDelegate.cpp`), `onRawDataFrameReceived` will start to receive callbacks.
  - I'm using `onRawDataFrameReceived` to save the YUV420 buffer into a file. You will need to use a converter such as ffmpeg to convert this YUV file into a playable mp4 or other playable video file. Do note that if you encounter corruption in your playable file, you might not be processing the YUV420 buffer according to standards. Do note that there might be varied resolution returned from the callback. Do not mixed them into the same saved file without processing. You will encounter corrupted file in playback. YUV processing is out of scope for this sample app.


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
 

## Resolving Conversion Errors

ffmpeg -f rawvideo -pix_fmt yuv420p -video_size 1280*720 -framerate 25 -i output.yuv -f mp4 output.mp4

What are the specs of the returned YUV buffer?
The resolution can range from 1920x1080, 1280x720, 640x360 etc....

What are some libraries I need to include in the main class? 
you will need to `#include <meeting_recording_interface.h>`

What can i use to playback or convert the YUV file into something playable?

Why does my app crash after running for a while?

Do note that YUV raw buffers are huge in size, it is not recommended to save these raw buffer directly to disk. Do look up on how to use ffmpeg or opencv to convert these raw buffers into decently sized, playable video files.

