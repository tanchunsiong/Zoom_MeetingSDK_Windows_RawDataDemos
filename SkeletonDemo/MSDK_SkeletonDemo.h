#include <meeting_service_components/meeting_participants_ctrl_interface.h>

using namespace ZOOMSDK;
class SkeletonDemo : public IMeetingParticipantsCtrlEvent {

	
	virtual void onUserJoin(IList<unsigned int >* lstUserID, const zchar_t* strUserList = NULL);


	virtual void onUserLeft(IList<unsigned int >* lstUserID, const zchar_t* strUserList = NULL);


	virtual void onHostChangeNotification(unsigned int userId);


	virtual void onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid);


	virtual void onUserNamesChanged(IList<unsigned int>* lstUserID);

	
	virtual void onCoHostChangeNotification(unsigned int userId, bool isCoHost);

	virtual void onInvalidReclaimHostkey();

	
	virtual void onAllHandsLowered();

	
	virtual void onLocalRecordingStatusChanged(unsigned int user_id, RecordingStatus status);


	virtual void onAllowParticipantsRenameNotification(bool bAllow);

	
	virtual void onAllowParticipantsUnmuteSelfNotification(bool bAllow);


	virtual void onAllowParticipantsStartVideoNotification(bool bAllow);;

	
	virtual void onAllowParticipantsShareWhiteBoardNotification(bool bAllow);


	virtual void onRequestLocalRecordingPrivilegeChanged(LocalRecordingRequestPrivilegeStatus status);

	virtual void onAllowParticipantsRequestCloudRecording(bool bAllow);
	
	virtual void onInMeetingUserAvatarPathUpdated(unsigned int userID);

	virtual void onParticipantProfilePictureStatusChange(bool bHidden);

	virtual void onFocusModeStateChanged(bool bEnabled);

	virtual void onFocusModeShareTypeChanged(FocusModeShareType type);
};