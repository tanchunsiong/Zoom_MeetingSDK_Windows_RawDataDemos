#include "windows.h"
#include "rawdata/rawdata_audio_helper_interface.h"
#include "VirtualAudioSource.h"
#include "zoom_sdk_def.h" 
#include <iostream>
#include <cstdint>

#include <fstream>
#include <cstring>
#include <cstdio>

#include <vector>
#include <thread>

using namespace std;
using namespace ZOOM_SDK_NAMESPACE;

int audio_play_flag = -1;



void PlayAudioFileToVirtualMic(IZoomSDKAudioRawDataSender* audio_sender, string audio_source)
{


	// execute in a thread.
	while (audio_play_flag > 0 && audio_sender) {

		// Check if the file exists
		ifstream file(audio_source, ios::binary | ios::ate);
		if (!file.is_open()) {
			cout << "Error: File not found. Tried to open " << audio_source << endl;
			return;
		}

		// Get the file size
		int file_size = file.tellg();

		// Read the file into a buffer
		vector<char> buffer(file_size);
		file.seekg(0, ios::beg);
		file.read(buffer.data(), file_size);

		// Send the audio data to the virtual camera
		SDKError err = audio_sender->send(buffer.data(), buffer.size(), 44100);
		if (err != SDKERR_SUCCESS) {
			cout << "Error: Failed to send audio data to virtual camera. Error code: " << err << endl;
			return;
		}
		file.close();
		audio_play_flag = -1;
	}
	
}

/// \brief Callback for virtual audio mic to do some initialization.
/// \param pSender, You can send audio data based on this object, see \link IZoomSDKAudioRawDataSender \endlink.
void VirtualAudioSource::onMicInitialize(IZoomSDKAudioRawDataSender* pSender) {
	//pSender->send();
	pSender_=pSender;
	printf("OnMicInitialize\n");
}

/// \brief Callback for virtual audio mic can send raw data with 'pSender'.
 void VirtualAudioSource::onMicStartSend() {
 
	 printf("onMicStartSend\n");


		 cout << "onStartSend" << endl;
	 if (pSender_ && audio_play_flag != 1) {
		 while (audio_play_flag > -1) {}
		 audio_play_flag = 1;
		 std::string audio_source_ = "pcm1644m.wav";
		 thread(PlayAudioFileToVirtualMic,pSender_, audio_source_).detach();
		
	 }
 }

/// \brief Callback for virtual audio mic should stop send raw data.
 void VirtualAudioSource::onMicStopSend() {
	 printf("onMicStopSend\n");

	
	 audio_play_flag = 0;
 }
/// \brief Callback for virtual audio mic is uninitialized.
 void VirtualAudioSource::onMicUninitialized() {
	 cout << "onUninitialized" << endl;
	 pSender_ = nullptr;
 }

