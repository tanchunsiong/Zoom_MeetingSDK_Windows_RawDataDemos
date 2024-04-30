#include "MeetingParticipantsCtrlEventListener.h"
#include <iostream>

using namespace std;

MeetingParticipantsCtrlEventListener::MeetingParticipantsCtrlEventListener(void (*onIsHost)(), void (*onIsCoHost)(), void (*onIsGivenRecordingPermission)())
{
	onIsHost_ = onIsHost;
	onIsCoHost_ = onIsCoHost;
	onIsGivenRecordingPermission_ = onIsGivenRecordingPermission;
}

/// \brief Callback event of notification of users who are in the meeting.
	/// \param lstUserID List of the user ID. 
	/// \param strUserList List of user in json format. This function is currently invalid, hereby only for reservations.
void MeetingParticipantsCtrlEventListener::onUserJoin(IList<unsigned int >* lstUserID, const wchar_t* strUserList) {}

/// \brief Callback event of notification of user who leaves the meeting.
/// \param lstUserID List of the user ID who leaves the meeting.
/// \param strUserList List of the user in json format. This function is currently invalid, hereby only for reservations.
void MeetingParticipantsCtrlEventListener::onUserLeft(IList<unsigned int >* lstUserID, const wchar_t* strUserList) {}

/// \brief Callback event of notification of the new host. 
/// \param userId Specify the ID of the new host. 
void MeetingParticipantsCtrlEventListener::onHostChangeNotification(unsigned int userId) {
	if (onIsHost_)onIsHost_();
}

/// \brief Callback event of changing the state of the hand.
/// \param bLow TRUE indicates to put down the hand, FALSE indicates to raise the hand. 
/// \param userid Specify the user ID whose status changes.
void MeetingParticipantsCtrlEventListener::onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid) {}

/// \brief Callback event of changing the screen name. 
/// \param userId list Specify the users ID whose status changes.
void MeetingParticipantsCtrlEventListener::onUserNamesChanged(IList<unsigned int>* lstUserID) {}

/// \brief Callback event of changing the co-host.
/// \param userId Specify the user ID whose status changes. 
/// \param isCoHost TRUE indicates that the specified user is co-host.
void MeetingParticipantsCtrlEventListener::onCoHostChangeNotification(unsigned int userId, bool isCoHost) {
	if (onIsCoHost_)onIsCoHost_();
}

/// \brief Callback event of invalid host key.
void MeetingParticipantsCtrlEventListener::onInvalidReclaimHostkey() {}

/// \brief Callback event of the host calls the lower all hands interface, the host/cohost/panelist will receive this callback.
void MeetingParticipantsCtrlEventListener::onAllHandsLowered() {}

/// \brief Callback event that the status of local recording changes.
/// \param userId Specify the user ID whose status changes. 
/// \param status Value of recording status. For more details, see \link RecordingStatus \endlink enum.
void MeetingParticipantsCtrlEventListener::onLocalRecordingStatusChanged(unsigned int user_id, RecordingStatus status) {}

/// \brief Callback event that lets participants rename themself.
/// \param bAllow True allow. If false, participants may not rename themselves.
void MeetingParticipantsCtrlEventListener::onAllowParticipantsRenameNotification(bool bAllow) {}

/// \brief Callback event that lets participants unmute themself.
/// \param bAllow True allow. If false, participants may not rename themselves.
void MeetingParticipantsCtrlEventListener::onAllowParticipantsUnmuteSelfNotification(bool bAllow) {}

/// \brief Callback event that lets participants start a video.
/// \param bAllow True allow. If false, disallow.
void MeetingParticipantsCtrlEventListener::onAllowParticipantsStartVideoNotification(bool bAllow) {}

/// \brief Callback event that lets participants share a new whiteboard.
/// \param bAllow True allow. If false, participants may not share new whiteboard.
void MeetingParticipantsCtrlEventListener::onAllowParticipantsShareWhiteBoardNotification(bool bAllow) {}

/// \brief Callback event that the request local recording privilege changes.
/// \param status Value of request local recording privilege status. For more details, see \link LocalRecordingRequestPrivilegeStatus \endlink enum.
void MeetingParticipantsCtrlEventListener::onRequestLocalRecordingPrivilegeChanged(LocalRecordingRequestPrivilegeStatus status) {}



void MeetingParticipantsCtrlEventListener::onAllowParticipantsRequestCloudRecording(bool bAllow) {}


/// \brief Callback event that the user avatar path is updated in the meeting.
/// \param userID Specify the user ID whose avatar updated. 
void MeetingParticipantsCtrlEventListener::onInMeetingUserAvatarPathUpdated(unsigned int userID) {}


/// \brief Callback event that participant profile status change.
/// \param bHide true means hide participant profile picture, false means show participant profile picture. 
void MeetingParticipantsCtrlEventListener::onParticipantProfilePictureStatusChange(bool bHidden) {}
/// \brief Callback event that focus mode changed by host or co-host.
/// \param bEnabled True means the focus mode change to on. Otherwise off.
void MeetingParticipantsCtrlEventListener::onFocusModeStateChanged(bool bEnabled) {}
/// \brief Callback event that that focus mode share type changed by host or co-host.
/// \param type Share type change.
void MeetingParticipantsCtrlEventListener::onFocusModeShareTypeChanged(FocusModeShareType type) {}

