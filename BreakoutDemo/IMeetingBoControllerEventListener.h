#pragma once
#include <windows.h>
#include "zoom_sdk.h"
#include <meeting_service_components/meeting_breakout_rooms_interface_v2.h>

using namespace ZOOMSDK;

class IMeetingBOControllerEventListener :
	public IMeetingBOControllerEvent
{
	
public:
	IMeetingBOControllerEventListener();

	/// \brief To notify that you has creator right. 
	/// \param pCreatorObj, the pointer of creator object. For more details, see \link IBOCreator \endlink enum.
	virtual void onHasCreatorRightsNotification(IBOCreator* pCreatorObj);

	/// \brief To notify that you has admin right. 
	/// \param pAdminObj, the pointer of admin object. For more details, see \link IBOAdmin \endlink enum.
	virtual void onHasAdminRightsNotification(IBOAdmin* pAdminObj);

	/// \brief To notify that you has assistant right. 
	/// \param pAssistantObj, the pointer of assistant object. For more details, see \link IBOAssistant \endlink enum.
	virtual void onHasAssistantRightsNotification(IBOAssistant* pAssistantObj);

	/// \brief To notify that you has attendee right. 
	/// \param pAttendeeObj, the pointer of attendee object. For more details, see \link IBOAttendee \endlink enum.
	virtual void onHasAttendeeRightsNotification(IBOAttendee* pAttendeeObj);

	/// \brief To notify that you has data right. 
	/// \param pDataHelperObj, the pointer of data helper object. For more details, see \link IBOData \endlink enum.
	virtual void onHasDataHelperRightsNotification(IBOData* pDataHelperObj);

	/// \brief To notify that you lost creator right. 
	virtual void onLostCreatorRightsNotification();

	/// \brief To notify that you lost admin right. 
	virtual void onLostAdminRightsNotification();

	/// \brief To notify that you lost assistant right. 
	virtual void onLostAssistantRightsNotification();

	/// \brief To notify that you lost attendee right. 
	virtual void onLostAttendeeRightsNotification();

	/// \brief To notify that you lost data right. 
	virtual void onLostDataHelperRightsNotification();

	/// \brief To notify that you receive a broadcast message. 
	/// \param strMsg, the message content.
	/// \param nSenderID, the SenderID.
	virtual void onNewBroadcastMessageReceived(const zchar_t* strMsg, unsigned int nSenderID, const zchar_t* strSenderName);

	/// \brief When BOOption.countdown_seconds != BO_STOP_NOT_COUNTDOWN, host stop BO and all users receive the event.
	/// \param nSeconds, the countdown seconds.
	virtual void onBOStopCountDown(unsigned int nSeconds);

	/// \brief When you are in BO, host invite you return to main session, you will receive the event.
	/// \param strName, the host name.
	virtual void onHostInviteReturnToMainSession(const zchar_t* strName, IReturnToMainSessionHandler* handler);

	/// \brief When host change the BO status, all users receive the event.
	/// \param eStatus, current status of BO.
	virtual void onBOStatusChanged(BO_STATUS eStatus);

	/// \brief Whenever the host switches you to another BO while you are assigned but haven't joined the BO, you will receive this event.
	/// \param strNewBOName The new BO name.
	/// \param strNewBOID The new BO ID. If the current user is IBOAttendee, then the 2nd parameter strNewBOID will return NULL.
	virtual void onBOSwitchRequestReceived(const zchar_t* strNewBOName, const zchar_t* strNewBOID);

	/// \brief The status of broadcasting voice to BO has been changed.
	/// \param bStart true for start and false for stop.
	virtual void onBroadcastBOVoiceStatus(bool bStart);

	/// \brief You will receive this event when you are in a breakout room and someone shares from the main session to the breakout room.
	/// \param iSharingID The sharing ID.
	/// \param status The sharing status. For more details, see \link SharingStatus \endlink enum.
	/// \param pShareAction The pointer of share action object. For more details, see \link IShareAction \endlink.
	/// \remarks Valid for user custom interface mode only.
	virtual void onShareFromMainSession(const unsigned int iSharingID, SharingStatus status, IShareAction* pShareAction);
};

