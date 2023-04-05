# Meeting SDK for Windows - Send Audio Raw Data

A Windows C++ Application demonstrate Zoom Meeting SDK sending audio raw data to a Zoom Meeting.

# Install vcpkg for adding dependency libs.
You might need to use Powershell (as administrator) or Windows Terminal to execute the sh script files
```
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


//To be completed




## Open and Run Project

Open "MSDK_SendAudioRawData.vcxproj" file from Visual Studio 2022.

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

  and reinstall

  ```
  ./vcpkg install jsoncpp
  ./vcpkg install opencv
  ```

  There are some errors about opencv being unable to find certain libraries?

  They should not affect the basic functionality of this sample, but you can do a list of additional opencv libraries to install
  ```
  ./cvpkg list opencv
  ```

  to install addition libraries, do something like this. This might take around 1 hour, depending on the number of libraries you install
  ```
  ./vcpkg install opencv[contrib,ffmpeg,nonfree,opengl,openmp,world]
  ```
## Getting Started

The main method, or main entry point of this application is at `MSDK_SendAudioRawData.cpp`

From a high level point of view it will do the below

- Join a meeting
- Wait for callback or status update. There are some prerequistes before you can get audio raw data. The `CanIStartRecording()` method helps to check if you have fulfilled these requirements
  - You need to be in-meeting. This is the status when you have fully joined a meeting.
- Create a `audio_source` object which implements `IZoomSDKAudioRawDataDelegate`. This is used to receive callbacks from the SDK is ready for you to send raw audio.
  - Call `GetAudioRawdataHelper` to get the `IZoomSDKAudioRawDataHelper` object. This is used to set the external audio source to our virtual `audio_source`
  - Call `audioHelper->setExternalAudioSource(audio_source)` to set the external audio source to our virtual `audio_source`
- Thereafter, you should be able to start getting callback in `VirtualAudioSource.cpp`, `onMicStartSend()`
  - I'm using `PlayAudioFileToVirtualMic` to read a PCM file into a buffer, and calling `audio_sender->send` to send it to the Zoom Meeting.

# Upgrading Guide

You will need to download the latest Meeting SDK Windows for c++ from marketplace.zoom.us

Replace the files in the folder `SDK` with those found in the downloaded files from marketplace.zoom.us

You will need to ensure any missing abstract classes are implemented etc... before you can compile and upgrade to a newer SDK version.

Some classes might need additional libraries, depending on your development environment, example...
```
#include <cstdint>
#include <windows.h>
```
# Resolving Errors

What are the specs of the  PCM audio?
Mono, 16 bits and varied bitrate supported. I'm using 44100khz

