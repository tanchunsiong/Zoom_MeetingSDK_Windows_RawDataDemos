#include "MeetingServiceEventListener.h"
#include <rawdata/zoom_rawdata_api.h>
#include <iostream>

using namespace std;

MeetingServiceEventListener::MeetingServiceEventListener(void (*onMeetingStarts)(), void (*onMeetingEnds)(), void (*onInMeeting)())
{
	onMeetingEnds_ = onMeetingEnds;
	onMeetingStarts_ = onMeetingStarts;
	onInMeeting_ = onInMeeting;
}

void MeetingServiceEventListener::onMeetingStatusChanged(MeetingStatus status, int iResult)
{
	std::cout << "onMeetingStatusChanged: " << status << ", iResult: " << iResult << endl;
	switch (status)
	{
	case MEETING_FAIL_MEETING_NOT_EXIST:
		printf("Meeting doesn't exitst, check your Meeting Number.\n");
		break;

	case MEETING_STATUS_INMEETING:
		printf("onMeetingStatusChanged() In Meeting.\n");
		if (onInMeeting_) onInMeeting_();

		break;
	}

	if (status == MEETING_STATUS_ENDED && onMeetingEnds_) onMeetingEnds_();
}

void MeetingServiceEventListener::onMeetingStatisticsWarningNotification(StatisticsWarningType type)
{
	std::cout << "onMeetingStatisticsWarningNotification, type: " << type << endl;
}

void MeetingServiceEventListener::onMeetingParameterNotification(const MeetingParameter* meeting_param)
{
	std::cout << "onMeetingParameterNotification" <<endl;
	if (onMeetingStarts_) onMeetingStarts_();
}


void MeetingServiceEventListener::onSuspendParticipantsActivities()
{
	std::cout << "onSuspendParticipantsActivities" << endl;
}

void MeetingServiceEventListener::onAICompanionActiveChangeNotice(bool bActive)
{
}

