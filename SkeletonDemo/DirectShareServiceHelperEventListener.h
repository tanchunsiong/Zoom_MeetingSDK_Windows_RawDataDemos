#pragma once
#include <windows.h>
#include <auth_service_interface.h>
#include "direct_share_helper_interface.h"


using namespace ZOOMSDK;
class DirectShareServiceHelperEventListener :
	public IDirectShareServiceHelperEvent
{
	
		void (*onMeetingEnds_)();
		void (*onMeetingStarts_)();
		void (*onInMeeting_)();
public:
	virtual void OnDirectShareStatusUpdate(DirectShareStatus status, IDirectShareViaMeetingIDOrPairingCodeHandler* handler);

	//only used for customui mode
	virtual void OnDirectShareSpecifyContent(IDirectShareSpecifyContentHandler* handler);
};

