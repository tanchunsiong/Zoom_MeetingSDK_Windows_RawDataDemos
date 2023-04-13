#include "windows.h"
#include "rawdata/rawdata_video_source_helper_interface.h"
#include "VideoSource.h"
#include "zoom_sdk_def.h" 
#include <iostream>
#include <cstdint>

#include <fstream>
#include <cstring>
#include <cstdio>

using namespace std;
using namespace ZOOM_SDK_NAMESPACE;


void VideoSource::onRawDataFrameReceived(YUVRawDataI420* data)
{
	cout << "onRawDataFrameReceived." << endl;
	// Open the file for writing
	ofstream outputFile("output.yuv", ios::out | ios::binary | ios::app);
	if (!outputFile.is_open())
	{
		cout << "Error opening file." << endl;
		return;
	}
	cout << "width." << data->GetStreamWidth() << endl;
	cout << "height." << data->GetStreamHeight() << endl;

	char* _data = new char[data->GetStreamHeight() * data->GetStreamWidth() * 3 / 2];
	
	memset(_data, 0, data->GetStreamHeight() * data->GetStreamWidth() * 3 / 2);

	// Copy Y buffer
	memcpy(_data, data->GetYBuffer(), data->GetStreamHeight() * data->GetStreamWidth());

	// Copy U buffer
	size_t loc = data->GetStreamHeight() * data->GetStreamWidth();
	memcpy(&_data[loc], data->GetUBuffer(), data->GetStreamHeight() * data->GetStreamWidth() / 4);


	// Copy V buffer
	loc = (data->GetStreamHeight() * data->GetStreamWidth()) + (data->GetStreamHeight() * data->GetStreamWidth() / 4);
	memcpy(&_data[loc], data->GetVBuffer(), data->GetStreamHeight() * data->GetStreamWidth() / 4);

	

	//outputFile.write((char*)data->GetBuffer(), data->GetBufferLen());
	// Write the Y plane
	outputFile.write(_data, data->GetStreamHeight() * data->GetStreamWidth() * 3/2);



	// Close the file
	outputFile.close();
	outputFile.flush();
	//cout << "YUV420 buffer saved to file." << endl;
}
void VideoSource::onRawDataStatusChanged(RawDataStatus status)
{
	cout << "onRawDataStatusChanged." << endl;
}

void VideoSource::onRendererBeDestroyed()
{
	cout << "onRendererBeDestroyed ." << endl;
}
