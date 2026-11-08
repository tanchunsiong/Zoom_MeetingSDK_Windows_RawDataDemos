#pragma once

#include <meeting_service_interface.h>
#include <meeting_service_components/meeting_audio_interface.h>
#include <meeting_service_components/meeting_closedcaption_interface.h>
#include <meeting_service_components/meeting_participants_ctrl_interface.h>

using namespace ZOOMSDK;

class MeetingServiceEventAdapter : public IMeetingServiceEvent
{
public:
	virtual void onUserNetworkStatusChanged(MeetingComponentType type, ConnectionQuality level, unsigned int userId, bool uplink) {}

#if defined(WIN32)
	virtual void onAppSignalPanelUpdated(IMeetingAppSignalHandler* pHandler) {}
#endif
};

class MeetingParticipantsCtrlEventAdapter : public IMeetingParticipantsCtrlEvent
{
public:
	virtual void onGrantCoOwnerPrivilegeChanged(bool canGrantOther) {}
};

class ClosedCaptionControllerEventAdapter : public IClosedCaptionControllerEvent
{
public:
	virtual void onSpokenLanguageChanged(ILiveTranscriptionLanguage* spokenLanguage) {}
};
