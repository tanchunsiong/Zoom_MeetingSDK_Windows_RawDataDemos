
//SendShareScreenRawData

#include <windows.h>
#include <string>
#include "rawdata\rawdata_share_source_helper_interface.h"

using namespace std;
using namespace ZOOMSDK;

class ZoomSDKShareSource :
	public IZoomSDKShareSource
{
private:
	IZoomSDKShareSender* share_sender_;
	std::string share_source_;
protected:

	virtual void onStartSend(IZoomSDKShareSender* pSender);
	virtual void onStopSend();

public:
	ZoomSDKShareSource(std::string share_source);
};
