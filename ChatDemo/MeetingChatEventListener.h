#pragma once
#include <windows.h>
#include "zoom_sdk.h"
#include <meeting_service_components/meeting_chat_interface.h> //CaptionDemo

using namespace std;
using namespace ZOOMSDK;

class MeetingChatEventListener :
	public IMeetingChatCtrlEvent
{
public:
	MeetingChatEventListener();

	/// \brief Chat message callback. This function is used to inform the user once received the message sent by others.
/// \param chatMsg An object pointer to the chat message.
/// \param content A pointer to the chat message in json format. This parameter is currently invalid, hereby only for reservations. 
	virtual void onChatMsgNotifcation(IChatMsgInfo* chatMsg, const zchar_t* content = NULL);

	/// \brief The authority of chat changes callback. This function is used to inform the user when the authority of chat changes in the meeting or webinar.
	/// \param status_ The chat status. For more details, see \link ChatStatus \endlink.
	virtual void onChatStatusChangedNotification(ChatStatus* status_);

	/// \brief Chat message be deleted callback. This function is used to inform the user host/myself the message be deleted.
	/// \param MsgID is the id of the deleted message.
	/// \param deleteBy Indicates by whom the message was deleted.
	virtual void onChatMsgDeleteNotification(const zchar_t* msgID, SDKChatMessageDeleteType deleteBy);

	virtual void onShareMeetingChatStatusChanged(bool isStart);

	/// \brief Invoked when start send file.
	/// \param sender The class to send file object.
	virtual void onFileSendStart(ISDKFileSender* sender);

	/// \brief Invoked when receiving a file from another user.
	/// \param receiver The class to receive the file object.
	virtual void onFileReceived(ISDKFileReceiver* receiver);

	/// \brief Invoked when send or receive file status change.
	/// \param info The class to basic transfer information.
	virtual void onFileTransferProgress(SDKFileTransferInfo* info);

};