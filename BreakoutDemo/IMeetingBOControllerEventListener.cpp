#include "IMeetingBOControllerEventListener.h"

using namespace std;


IMeetingBOControllerEventListener::IMeetingBOControllerEventListener()
{
	
}


/// \brief To notify that you has creator right. 
	/// \param pCreatorObj, the pointer of creator object. For more details, see \link IBOCreator \endlink enum.
void IMeetingBOControllerEventListener::onHasCreatorRightsNotification(IBOCreator* pCreatorObj){}

/// \brief To notify that you has admin right. 
/// \param pAdminObj, the pointer of admin object. For more details, see \link IBOAdmin \endlink enum.
void IMeetingBOControllerEventListener::onHasAdminRightsNotification(IBOAdmin* pAdminObj){}

/// \brief To notify that you has assistant right. 
/// \param pAssistantObj, the pointer of assistant object. For more details, see \link IBOAssistant \endlink enum.
void IMeetingBOControllerEventListener::onHasAssistantRightsNotification(IBOAssistant* pAssistantObj){}

/// \brief To notify that you has attendee right. 
/// \param pAttendeeObj, the pointer of attendee object. For more details, see \link IBOAttendee \endlink enum.
void IMeetingBOControllerEventListener::onHasAttendeeRightsNotification(IBOAttendee* pAttendeeObj){}

/// \brief To notify that you has data right. 
/// \param pDataHelperObj, the pointer of data helper object. For more details, see \link IBOData \endlink enum.
void IMeetingBOControllerEventListener::onHasDataHelperRightsNotification(IBOData* pDataHelperObj){}

/// \brief To notify that you lost creator right. 
void IMeetingBOControllerEventListener::onLostCreatorRightsNotification(){}

/// \brief To notify that you lost admin right. 
void IMeetingBOControllerEventListener::onLostAdminRightsNotification(){}

/// \brief To notify that you lost assistant right. 
void IMeetingBOControllerEventListener::onLostAssistantRightsNotification(){}

/// \brief To notify that you lost attendee right. 
void IMeetingBOControllerEventListener::onLostAttendeeRightsNotification(){}

/// \brief To notify that you lost data right. 
void IMeetingBOControllerEventListener::onLostDataHelperRightsNotification(){}

/// \brief To notify that you receive a broadcast message. 
/// \param strMsg, the message content.
/// \param nSenderID, the SenderID.
void IMeetingBOControllerEventListener::onNewBroadcastMessageReceived(const zchar_t* strMsg, unsigned int nSenderID, const zchar_t* strSenderName){}

/// \brief When BOOption.countdown_seconds != BO_STOP_NOT_COUNTDOWN, host stop BO and all users receive the event.
/// \param nSeconds, the countdown seconds.
void IMeetingBOControllerEventListener::onBOStopCountDown(unsigned int nSeconds){}

/// \brief When you are in BO, host invite you return to main session, you will receive the event.
/// \param strName, the host name.
void IMeetingBOControllerEventListener::onHostInviteReturnToMainSession(const zchar_t* strName, IReturnToMainSessionHandler* handler){}

/// \brief When host change the BO status, all users receive the event.
/// \param eStatus, current status of BO.
void IMeetingBOControllerEventListener::onBOStatusChanged(BO_STATUS eStatus){}

/// \brief Whenever the host switches you to another BO while you are assigned but haven't joined the BO, you will receive this event.
/// \param strNewBOName The new BO name.
/// \param strNewBOID The new BO ID. If the current user is IBOAttendee, then the 2nd parameter strNewBOID will return NULL.
void IMeetingBOControllerEventListener::onBOSwitchRequestReceived(const zchar_t* strNewBOName, const zchar_t* strNewBOID){}

/// \brief The status of broadcasting voice to BO has been changed.
/// \param bStart true for start and false for stop.
void IMeetingBOControllerEventListener::onBroadcastBOVoiceStatus(bool bStart){}

void IMeetingBOControllerEventListener::onShareFromMainSession(const unsigned int iSharingID, SharingStatus status, IShareAction* pShareAction)
{
}
