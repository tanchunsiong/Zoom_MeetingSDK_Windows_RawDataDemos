#include "windows.h"
#include "rawdata/rawdata_audio_helper_interface.h"
#include "AudioSource.h"
#include "zoom_sdk_def.h" 
#include <iostream>
#include <fstream>



using namespace std;
using namespace ZOOM_SDK_NAMESPACE;
void AudioSource::onOneWayAudioRawDataReceived(AudioRawData* audioRawData, uint32_t node_id)
{
	//std::cout << "Received onOneWayAudioRawDataReceived" << std::endl;
	//add your code here
}
void AudioSource::onMixedAudioRawDataReceived(AudioRawData* audioRawData)
{
	std::cout << "Received onMixedAudioRawDataReceived" << std::endl;
	//add your code here


	static ofstream pcmFile;
	pcmFile.open("audio.pcm", ios::out | ios::binary | ios::app);

	if (!pcmFile.is_open()) {
		cout << "Failed to open wave file" << endl;
		return;
	}
	try {
		// Write the audio data to the file
		pcmFile.write((char*)audioRawData->GetBuffer(), audioRawData->GetBufferLen());
		//std::cout << "buffer length: " << audioRawData->GetBufferLen() << std::endl;
		std::cout << "buffer : " << audioRawData->GetBuffer() << std::endl;

		// Close the wave file
		pcmFile.close();
		pcmFile.flush();
	}
	catch (exception e)
	{
		cout << "Failed to write wave file" << endl;
	}


}