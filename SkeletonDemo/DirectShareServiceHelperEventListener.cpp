#include "MeetingServiceEventListener.h"
#include <rawdata/zoom_rawdata_api.h>
#include <iostream>
#include "DirectShareServiceHelperEventListener.h"

using namespace std;

void DirectShareServiceHelperEventListener::OnDirectShareStatusUpdate(DirectShareStatus status, IDirectShareViaMeetingIDOrPairingCodeHandler* handler)
{
	if (status == DirectShareStatus::DirectShare_Need_MeetingID_Or_PairingCode) {
	
		handler->TryWithMeetingNumber(5648943370);
	}
}

void DirectShareServiceHelperEventListener::OnDirectShareSpecifyContent(IDirectShareSpecifyContentHandler* handler)
{

	handler->GetSupportedDirectShareType();
	handler->TryShareDesktop(L"1", true, true);
}

