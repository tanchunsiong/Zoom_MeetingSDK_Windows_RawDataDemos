# Zoom Meeting SDK for Windows Raw Data Demos


TO download the repo
`git clone https://github.com/tanchunsiong/MSDK_RawDataDemos.git`

Thereafter launch `MSDK_RawDataDemos.sln` from explorer or open up from Visual Studio.

Each sample folder will have a specific sample demo. (example GetAudioRawData, GetVideoRawData...)

Within each sample folder you will need 2 addition items
- 1. Create a file named `config.json` in each folder. Here are some parameters which are expected within `config.json`
```
{
  "sdk_jwt": "aaaa.bbbbb.ccccccc",
  "meeting_number": "123123123123",
  "passcode": "123123",
  "video_source": "Big_Buck_Bunny_720_10s_1MB.mp4",
  "zak": ""
}
```

- 2. In each sample folder, populate the folder named "SDK" with the Zoom Meeting SDK libraries. These are not included in the repo. They can be downloaded from the (Zoom SDK website)[marketplace.zoom.us]. The file/folder structure within the SDK folder should be as follows:
	- x64
	- x86
	- CHANGELOG.MD
	- OSS-LICENSE.txt
	- README.md
	- version.txt

