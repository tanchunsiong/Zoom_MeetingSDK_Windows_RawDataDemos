# Zoom Meeting SDK for Windows Raw Data Demos
Each folder will have a specific sample demo. (example GetAudioRawData, GetVideoRawData...)

Within each folder you will need 2 addition items
- a file named `config.json` . Here are some parameters which are expected within `config.json`
```
{
  "sdk_jwt": "aaaa.bbbbb.ccccccc",
  "meeting_number": "123123123123",
  "passcode": "123123",
  "video_source": "Big_Buck_Bunny_720_10s_1MB.mp4",
  "zak": ""
}
```


- populate the folder named "SDK" with the Zoom Meeting SDK libraries. These are not included in the repo. They can be downloaded from the (Zoom SDK website)[marketplace.zoom.us]. The file/folder structure within the SDK folder should be as follows:
	- x64
	- x86
	- CHANGELOG.MD
	- OSS-LICENSE.txt
	- README.md
	- version.txt

These projects are currently using Zoom Meeting SDK for Windows v5.14.5.15340