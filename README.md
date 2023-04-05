# Zoom Meeting SDK for Windows Raw Data Demos
Consolidation of 1 Solution with multiple raw data projects


Each folder will have a specific sample demo.
Within each of these folders you will need 2 addition items
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
- a folder named "SDK" with the Zoom SDK libraries. These are not included in the repo. They can be downloaded from the Zoom SDK website.
	- x64
	- x86
	- CHANGELOG.MD
	- OSS-LICENSE.txt
	- README.md
	- version.txt