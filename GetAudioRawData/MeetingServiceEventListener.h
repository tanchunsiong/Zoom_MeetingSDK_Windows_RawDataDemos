#pragma once
#include <windows.h>
#include "zoom_sdk.h"
#include <meeting_service_interface.h>

using namespace ZOOMSDK;

class MeetingServiceEventListener :
    public IMeetingServiceEvent
{
	void (*onMeetingEnds_)();
	void (*onMeetingStarts_)();
public:
	MeetingServiceEventListener(void (*onMeetingStarts_)(), void (*onMeetingEnds_)());
	/// \brief Meeting status changed callback.
	/// \param status The value of meeting. For more details, see \link MeetingStatus \endlink.
	/// \param iResult Detailed reasons for special meeting status.
	///If the status is MEETING_STATUS_FAILED, the value of iResult is one of those listed in MeetingFailCode enum. 
	///If the status is MEETING_STATUS_ENDED, the value of iResult is one of those listed in MeetingEndReason.
	virtual void onMeetingStatusChanged(MeetingStatus status, int iResult = 0);

	/// \brief Meeting statistics warning notification callback.
	/// \param type The warning type of the meeting statistics. For more details, see \link StatisticsWarningType \endlink.
	virtual void onMeetingStatisticsWarningNotification(StatisticsWarningType type);

	/// \brief Meeting parameter notification callback.
	/// \param meeting_param Meeting parameter. For more details, see \link MeetingParameter \endlink.
	/// \remarks The callback will be triggered right before the meeting starts. The meeting_param will be destroyed once the function calls end.
	virtual void onMeetingParameterNotification(const MeetingParameter* meeting_param);
};

