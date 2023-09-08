#pragma once
#include "windows.h"
#include <cstdint>
#include <iostream>
#include <cstdint>
#include "rawdata/rawdata_audio_helper_interface.h"
#include "zoom_sdk.h"
#include "zoom_sdk_raw_data_def.h"


using namespace std;
using namespace ZOOMSDK;

class ZoomSDKAudioRawDataDelegate :
	public IZoomSDKAudioRawDataDelegate
{
public:
	virtual void onMixedAudioRawDataReceived(AudioRawData* data_);
	virtual void onOneWayAudioRawDataReceived(AudioRawData* data_, uint32_t node_id);
};