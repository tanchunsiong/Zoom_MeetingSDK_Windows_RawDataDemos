#include "MeetingRecordingCtrlEventListener.h"

using namespace std;


MeetingRecordingCtrlEventListener::MeetingRecordingCtrlEventListener(void(*onIsGivenRecordingPermission)())
{
	onIsGivenRecordingPermission_ = onIsGivenRecordingPermission;
}

/// \brief Callback event that the status of my local recording changes.
/// \param status Value of recording status. For more details, see \link RecordingStatus \endlink enum.
void MeetingRecordingCtrlEventListener::onRecordingStatus(RecordingStatus status) {}
/// \brief Callback event that the status of cloud recording changes.
/// \param status Value of recording status. For more details, see \link RecordingStatus \endlink enum.
void MeetingRecordingCtrlEventListener::onCloudRecordingStatus(RecordingStatus status) {}

/// \brief Callback event that the recording authority changes.
/// \param bCanRec TRUE indicates to enable to record.
void MeetingRecordingCtrlEventListener::onRecordPrivilegeChanged(bool bCanRec) {

	if (bCanRec) {
		if (onIsGivenRecordingPermission_)onIsGivenRecordingPermission_();
	}
}

/// \brief Callback event that the status of request local recording privilege.
/// \param status Value of request local recording privilege status. For more details, see \link RequestLocalRecordingStatus \endlink enum.
void MeetingRecordingCtrlEventListener::onLocalRecordingPrivilegeRequestStatus(RequestLocalRecordingStatus status) {}

/// \brief Callback event for when the host responds to a cloud recording permission request
/// \param status Value of request host to start cloud recording response status. For more details, see \link RequestStartCloudRecordingStatus \endlink enum.
void MeetingRecordingCtrlEventListener::onRequestCloudRecordingResponse(RequestStartCloudRecordingStatus status) {}

/// \brief Callback event when a user requests local recording privilege.
/// \param handler A pointer to the IRequestLocalRecordingPrivilegeHandler. For more details, see \link IRequestLocalRecordingPrivilegeHandler \endlink.
void MeetingRecordingCtrlEventListener::onLocalRecordingPrivilegeRequested(IRequestLocalRecordingPrivilegeHandler* handler) {}

/// \brief Callback event received only by the host when a user requests to start cloud recording.
/// \param handler A pointer to the IRequestStartCloudRecordingHandler. For more details, see \link IRequestStartCloudRecordingHandler \endlink.
void MeetingRecordingCtrlEventListener::onStartCloudRecordingRequested(IRequestStartCloudRecordingHandler* handler) {}

/// \brief Callback event of ending the conversion to MP4 format.
/// \param bsuccess TRUE indicates to convert successfully. FALSE not.
/// \param iResult This value is used to save the error code only when the convert fails.
/// \param szPath If the conversion is successful, this value is used to save the path of the recording file. 
/// \remarks In order to trigger this callback correctly, you need call IMeetingConfiguration.EnableLocalRecordingConvertProgressBarDialog(false) before you start a meeting.
void MeetingRecordingCtrlEventListener::onRecording2MP4Done(bool bsuccess, int iResult, const zchar_t* szPath) {}

/// \brief Callback event of the process of the conversion to MP4 format.
/// \param iPercentage Percentage of conversion process. Range from ZERO(0) to ONE HUNDREAD(100).
/// \remarks In order to trigger this callback correctly, you need call IMeetingConfiguration.EnableLocalRecordingConvertProgressBarDialog(false) before you start a meeting.
void MeetingRecordingCtrlEventListener::onRecording2MP4Processing(int iPercentage) {}

/// \brief Callback event that the local recording source changes in the custom user interface mode.
/// \param layout_helper An object pointer to ICustomizedLocalRecordingLayoutHelper. For more details, see \link ICustomizedLocalRecordingLayoutHelper \endlink.
///The layout_helper won't be released till the call ends. The user needs to complete the related layout before the call ends. 
void MeetingRecordingCtrlEventListener::onCustomizedLocalRecordingSourceNotification(ICustomizedLocalRecordingLayoutHelper* layout_helper) {}

/// \param gracePeriodDate a point in time, in milliseconds, in UTC. You can use the cloud recording storage until the gracePeriodDate.
void MeetingRecordingCtrlEventListener::onCloudRecordingStorageFull(time_t gracePeriodDate) {
}

void MeetingRecordingCtrlEventListener::onEnableAndStartSmartRecordingRequested(IRequestEnableAndStartSmartRecordingHandler* handler)
{
}

void MeetingRecordingCtrlEventListener::onSmartRecordingEnableActionCallback(ISmartRecordingEnableActionHandler* handler)
{
}

