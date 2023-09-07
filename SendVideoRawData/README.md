
# Meeting SDK for Windows - Send Video Raw Data

A Windows C++ Application demonstrate Zoom Meeting SDK sending Video raw data to a Zoom Meeting.

# Install vcpkg for adding dependency libs.
You might need to use Powershell (as administrator) or Windows Terminal to execute the sh script files
```
cd ~
cd source
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

# Add dependency libs
opencv might not be necessary if you are not going to do video/image processing before saving.
opencv might not be necessary if you are just saving raw audio to file..
opencv will take a while (10 mins) to complete

```
./vcpkg install jsoncpp
./vcpkg install opencv 
```

# Clone the project source code

```
git clone https://github.com/tanchunsiong/MSDK_RawDataDemos
```

This project is in the SendVideoRawData folder

## Add a configuration file named `config.json`

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




## Open and Run Project

Open "MSDK_SendVideoRawData.vcxproj" file from Visual Studio 2022.

Hit F5 or click from menu "Debug" -> "Start Debugging" in x86 or x64 to launch the application.


## Error

if you are getting an error about not being able to open source json/json.h , include this in your

Visual Studio Project -> Properties. Under C/C++ ->General ->Additional Include Directories,

 ### x64
 C:\yourpath\whereyouinstalled\vcpkg\packages\jsoncpp_x64-windows\include
 
 or

 ### x86
 C:\yourpath\whereyouinstalled\vcpkg\packages\jsoncpp_x86-windows\include

 # Error

  what if i would like to use x64 environment?

  add this to your environment variable before installing openCV from vcpkg

  VCPKG_DEFAULT_TRIPLET = x64-windows

  you can use `setx VCPKG_DEFAULT_TRIPLET "x64-windows"` to set it via command line

  and reinstall by using the command below

  ```
  ./vcpkg install jsoncpp
  ./vcpkg install opencv
  ```

  There are some errors about opencv being unable to find certain libraries?

  They should not affect the basic functionality of this sample, but you can do a list of additional opencv libraries to install
  ```
  ./cvpkg list opencv
  ```

  to install addition libraries, do something like this. [Warning]This might take around 1 hour, depending on the number of libraries you install
  ```
  ./vcpkg install opencv[contrib,ffmpeg,nonfree,opengl,openmp,world]
  ```
## Getting Started

The main method, or main entry point of this application is at `MSDK_SendVideoRawData.cpp`

From a high level point of view it will do the below

- Join a meeting
- Wait for callback or status update. There are some prerequistes before you can get video raw data. The `prereqCheckForRawVideoSend()` method helps to check if you have fulfilled these requirements
  - `HasRawdataLicense()` <- this has been deprecated, you no longer need a license for raw data access.
  - You need to be in-meeting. This is the status when you have fully joined a meeting.
- Create virtual_camera_video_source, which is an implementation of IZoomSDKVideoSource
  - Call `GetRawdataVideoSourceHelper()` to get the IZoomSDKVideoSourceHelper interface. This interface is used to assigned / set a virtual camera video source.
  - Call `setExternalVideoSource(virtual_camera_video_source)` to set the virtual camera source.
- At this stage, you should be able to see the virtual camera in your video conferencing application. You can use this virtual camera to send video raw data to the meeting.
  - In `ZoomSDKVideoSource.cpp`, `onStartSend()`, you can implement your own logic to send video raw data to the meeting. I'm using an additional method `PlayVideoFileToVirtualCamera` which reads a video file, converts it to YUV420 buffer, and sens it via `sendVideoFrame` method.

# Upgrading Guide

You will need to download the latest Meeting SDK Windows for c++ from marketplace.zoom.us

Replace the files in the folder `SDK` with those found in the downloaded files from marketplace.zoom.us

You will need to ensure any missing abstract classes are implemented etc... before you can compile and upgrade to a newer SDK version.

Some classes might need additional libraries, depending on your development environment, example...
```
#include <cstdint>
#include <windows.h>
```

You might encounter these errors

Error	C3646	'GetAudioJoinType': unknown override specifier	
Error (active)	E0020	identifier "AudioType" is undefined	SkeletonDemo

In your meeting_participants_ctrl_interface.h, ensure you are add an addition reference to #include "meeting_service_components/meeting_audio_interface.h"

Rebuild and Run, it should resolve both errors above

# Resolving Errors


#adding curl, needed for webservice.cpp and webservice.h

./vcpkg install curl

add c++, general, addition include directories "C:\Users\$(UserName)\source\vcpkg\packages\curl_x64-windows\include" 