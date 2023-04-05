#pragma once
#include <windows.h>
#include <string>
#include "rawdata\rawdata_share_source_helper_interface.h"

constexpr auto WIDTH = 640;
constexpr auto HEIGHT = 480;

using namespace std;
using namespace ZOOMSDK;



class VirtualShareSource :
	public IZoomSDKShareSource
{
private:
	IZoomSDKShareSender* share_sender_;
	std::string share_source_;
protected:

	virtual void onStartSend(IZoomSDKShareSender* pSender);
	virtual void onStopSend();

public:
	VirtualShareSource(std::string share_source);
};
