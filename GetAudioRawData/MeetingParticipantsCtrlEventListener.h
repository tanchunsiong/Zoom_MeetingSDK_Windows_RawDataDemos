#pragma once
#include <windows.h>
#include "zoom_sdk.h"
#include <meeting_service_interface.h>
#include <meeting_service_components/meeting_audio_interface.h>
#include <meeting_service_components/meeting_participants_ctrl_interface.h>
using namespace ZOOMSDK;

class MeetingParticipantsCtrlEventListener :
	public IMeetingParticipantsCtrlEvent
{
	void (*onIsHost_)();
	void (*onIsCoHost_)();
	void (*onIsGivenRecordingPermission_)();
public:
	MeetingParticipantsCtrlEventListener(void (*onIsHost_)(), void (*onIsCoHost_)(), void (*onIsGivenRecordingPermission_)());
	/// \brief Callback event of notification of users who are in the meeting.
	/// \param lstUserID List of the user ID. 
	/// \param strUserList List of user in json format. This function is currently invalid, hereby only for reservations.
	virtual void onUserJoin(IList<unsigned int >* lstUserID, const wchar_t* strUserList = NULL);

	/// \brief Callback event of notification of user who leaves the meeting.
	/// \param lstUserID List of the user ID who leaves the meeting.
	/// \param strUserList List of the user in json format. This function is currently invalid, hereby only for reservations.
	virtual void onUserLeft(IList<unsigned int >* lstUserID, const wchar_t* strUserList = NULL);

	/// \brief Callback event of notification of the new host. 
	/// \param userId Specify the ID of the new host. 
	virtual void onHostChangeNotification(unsigned int userId);

	/// \brief Callback event of changing the state of the hand.
	/// \param bLow TRUE indicates to put down the hand, FALSE indicates to raise the hand. 
	/// \param userid Specify the user ID whose status changes.
	virtual void onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid);

	/// \brief Callback event of changing the screen name. 
	/// \param userId list Specify the users ID whose status changes.
	virtual void onUserNamesChanged(IList<unsigned int>* lstUserID);

	/// \brief Callback event of changing the co-host.
	/// \param userId Specify the user ID whose status changes. 
	/// \param isCoHost TRUE indicates that the specified user is co-host.
	virtual void onCoHostChangeNotification(unsigned int userId, bool isCoHost);

	/// \brief Callback event of invalid host key.
	virtual void onInvalidReclaimHostkey();

	/// \brief Callback event of the host calls the lower all hands interface, the host/cohost/panelist will receive this callback.
	virtual void onAllHandsLowered();

	/// \brief Callback event that the status of local recording changes.
	/// \param userId Specify the user ID whose status changes. 
	/// \param status Value of recording status. For more details, see \link RecordingStatus \endlink enum.
	virtual void onLocalRecordingStatusChanged(unsigned int user_id, RecordingStatus status);

	/// \brief Callback event that lets participants rename themself.
	/// \param bAllow True allow. If false, participants may not rename themselves.
	virtual void onAllowParticipantsRenameNotification(bool bAllow);

	/// \brief Callback event that lets participants unmute themself.
	/// \param bAllow True allow. If false, participants may not rename themselves.
	virtual void onAllowParticipantsUnmuteSelfNotification(bool bAllow);

	/// \brief Callback event that lets participants start a video.
	/// \param bAllow True allow. If false, disallow.
	virtual void onAllowParticipantsStartVideoNotification(bool bAllow);

	/// \brief Callback event that lets participants share a new whiteboard.
	/// \param bAllow True allow. If false, participants may not share new whiteboard.
	virtual void onAllowParticipantsShareWhiteBoardNotification(bool bAllow);


	/// \brief Callback event that the request local recording privilege changes.
	/// \param status Value of request local recording privilege status. For more details, see \link LocalRecordingRequestPrivilegeStatus \endlink enum.
	virtual void onRequestLocalRecordingPrivilegeChanged(LocalRecordingRequestPrivilegeStatus status);

	/// \brief Callback event that lets participants request that the host starts cloud recording.
	/// \param bAllow True allow. If false, disallow.
	virtual void onAllowParticipantsRequestCloudRecording(bool bAllow);

	/// \brief Callback event that the user avatar path is updated in the meeting.
	/// \param userID Specify the user ID whose avatar updated. 
	virtual void onInMeetingUserAvatarPathUpdated(unsigned int userID);

	/// \brief Callback event that participant profile status change.
	/// \param bHide true means hide participant profile picture, false means show participant profile picture. 
	virtual void onParticipantProfilePictureStatusChange(bool bHidden);

	/// \brief Callback event that focus mode changed by host or co-host.
	/// \param bEnabled True means the focus mode change to on. Otherwise off.
	virtual void onFocusModeStateChanged(bool bEnabled);

	/// \brief Callback event that that focus mode share type changed by host or co-host.
	/// \param type Share type change.
	virtual void onFocusModeShareTypeChanged(FocusModeShareType type);
};

