//SendVideoRawData


#include <windows.h>
#include <string>
#include "rawdata\rawdata_video_source_helper_interface.h"


using namespace std;
using namespace ZOOMSDK;

class ZoomSDKVideoSource :
	public IZoomSDKVideoSource
{
private:
	IZoomSDKVideoSender* video_sender_;
	std::string video_source_;
protected:
	virtual	void onInitialize(IZoomSDKVideoSender* sender, IList<VideoSourceCapability >* support_cap_list, VideoSourceCapability& suggest_cap);
	virtual void onPropertyChange(IList<VideoSourceCapability >* support_cap_list, VideoSourceCapability suggest_cap);
	virtual void onStartSend();
	virtual void onStopSend();
	virtual void onUninitialized();
public:
	ZoomSDKVideoSource(std::string video_source);
};

