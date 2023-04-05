# MSDK_GetAudioRawData

A Windows C++ Application demostrate Zoom Meeting SDK receiving audio raw data as user's audio source to a Zoom Meeting.

## Install vcpkg for adding dependency libs.
## You might need to use Powershell (as administrator) or Windows Terminal to execute the sh script files
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
```

## Add dependency libs
## opencv might not be necessary if you are not going to do video/image processing before saving.
## opencv might not be necessary if you are just saving raw audio to file..
## opencv will take a while (10 mins) to complete

```
./vcpkg install jsoncpp
./vcpkg install opencv
```

## Clone the project source code

```
git clone https://git.zoom.us/chunsiong.tan/MSDK_GetAudioRawData
```

## Add config to config.json

```
{
  "sdk_jwt": "<your_sdk_jwt>",
  "meeting_number": <meeting_number_to_join>,
  "passcode": "<passcode>",
  "video_source": "Big_Buck_Bunny_1080_10s_1MB.mp4"
}
```

The app will try to join the meeting follow the Meeting Number you specified in the config.json. Specify a video source for sending to the meeting as a vitual video source. The video source could be a file path or a stream url. A video file name, "Big_Buck_Bunny_1080_10s_1MB.mp4" is set by default, which is video file's name included in project source code.

## Open and Run Project

Open "MSDK_SendVideoRawData.vcxproj" file from Visual Studio 2022.

Hit F5 or click from menu "Debug" -> "Start Debugging" in x86 to launch the application.


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
 


#TODO change this description

1. add AudioSource.h header file
2. add a delegate AudioSource.cpp file
3. explain the folder structure, upgrading guide
4. resolving errors (a strategy)
5. requirements to get raw audio
6. audio guide

you might need to in some files if it complains
#include <cstdint>
#include <windows.h>

in the main class you will need to include
#include <meeting_recording_interface.h>