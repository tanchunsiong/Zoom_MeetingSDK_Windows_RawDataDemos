#include "windows.h"
#include <iostream>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <vector>

#include "rawdata/rawdata_audio_helper_interface.h"
#include "ZoomSDKVirtualAudioMicEvent.h"
#include "zoom_sdk_def.h" 

#include <thread>

using namespace std;
using namespace ZOOM_SDK_NAMESPACE;

int audio_play_flag = -1;

void PlayAudioFileToVirtualMic2(IZoomSDKAudioRawDataSender* audio_sender, string audio_source)
{
	while (audio_play_flag > 0 && audio_sender) {

		// Open the WAV file in binary mode
		std::ifstream file(audio_source, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open file " << audio_source << std::endl;

		}

		file.seekg(0, std::ios::end);
		std::streamsize fileSize = file.tellg();
		// Skip the WAV header (first 44 bytes)
		file.seekg(44, std::ios::beg);

		// Set fixed parameters for the WAV file
		const int sampleRate = 48000;  // 44.1kHz
		const int bitsPerSample = 16;  // 16 bits per sample
		const int channels = 1;        // Mono

		std::cout << "WAV File Details: " << sampleRate << " Hz, " << bitsPerSample << " bits, " << channels << " channel" << std::endl;


		// Calculate the duration of the WAV file
		std::streamsize audioDataSize = fileSize - 44; // Subtract the header size
		double durationSeconds = static_cast<double>(audioDataSize) / (sampleRate * channels * (bitsPerSample / 8));


		// Send the audio data in chunks
		const size_t chunkSize = 640;  // Send in 640 byte chunks
		std::vector<char> buffer(chunkSize);
		std::vector<std::thread> send_threads;  // To store threads

		while (file.good()) {
			file.read(buffer.data(), chunkSize);
			std::streamsize bytesRead = file.gcount();

			//this loop is to send the audio data in chunks to Zoom, but is has not finished playing to all recipients
			if (bytesRead > 0) {
				// Send the chunk of audio data to the virtual mic
				audio_sender->send(buffer.data(), bytesRead, sampleRate, ZoomSDKAudioChannel_Mono);
			}
		}

		file.close();
		std::cout << "Finished sending WAV file." << std::endl;

		//print the duration seconds
		std::cout << "Duration: " << durationSeconds << " seconds" << std::endl;

		// Sleep for the calculated duration to ensure the audio finishes playing
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(durationSeconds * 1000)));


		//audio_play_flag = -1;
	}

}
void PlayAudioFileToVirtualMic(IZoomSDKAudioRawDataSender* audio_sender, string audio_source)
{


	// execute in a thread.
	while (audio_play_flag > 0 && audio_sender) {

		// Check if the file exists
		ifstream file(audio_source, ios::binary | ios::ate);
		if (!file.is_open()) {
			std::cout << "Error: File not found. Tried to open " << audio_source << std::endl;
			return;
		}

		// Get the file size
		int file_size = file.tellg();

		// Read the file into a buffer
		vector<char> buffer(file_size);
		file.seekg(0, ios::beg);
		file.read(buffer.data(), file_size);

		// Send the audio data to the virtual mic
		SDKError err = audio_sender->send(buffer.data(), buffer.size(), 44100);
		if (err != SDKERR_SUCCESS) {
			std::cout << "Error: Failed to send audio data to virtual mic. Error code: " << err << std::endl;
			return;
		}
		file.close();
		audio_play_flag = -1;
	}

}

/// \brief Callback for virtual audio mic to do some initialization.
/// \param pSender, You can send audio data based on this object, see \link IZoomSDKAudioRawDataSender \endlink.
void ZoomSDKVirtualAudioMicEvent::onMicInitialize(IZoomSDKAudioRawDataSender* pSender) {
	//pSender->send();
	pSender_ = pSender;
	printf("OnMicInitialize\n");
}

/// \brief Callback for virtual audio mic can send raw data with 'pSender'.
void ZoomSDKVirtualAudioMicEvent::onMicStartSend() {

	printf("onMicStartSend\n");


	std::cout << "onStartSend" << std::endl;
	//if (pSender_ && audio_play_flag != 1) {
		
		audio_play_flag = 1;
		std::string audio_source_ = "pcm1644m.wav";
		thread(PlayAudioFileToVirtualMic2, pSender_, audio_source_).detach();

	//}
}

/// \brief Callback for virtual audio mic should stop send raw data.
void ZoomSDKVirtualAudioMicEvent::onMicStopSend() {
	printf("onMicStopSend\n");


	audio_play_flag = 0;
}
/// \brief Callback for virtual audio mic is uninitialized.
void ZoomSDKVirtualAudioMicEvent::onMicUninitialized() {
	std::cout << "onUninitialized" << std::endl;
	pSender_ = nullptr;
}
