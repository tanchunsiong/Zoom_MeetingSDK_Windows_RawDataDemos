#include "MeetingChatEventListener.h"
#include <iostream>

using namespace std;
using namespace ZOOMSDK;

MeetingChatEventListener::MeetingChatEventListener() {

}

void MeetingChatEventListener::onChatMsgNotifcation(IChatMsgInfo* chatMsg, const zchar_t* content)
{
	std::cout << content << endl;
}

void MeetingChatEventListener::onChatStatusChangedNotification(ChatStatus* status_)
{
}

void MeetingChatEventListener::onChatMsgDeleteNotification(const zchar_t* msgID, SDKChatMessageDeleteType deleteBy)
{
}

void MeetingChatEventListener::onShareMeetingChatStatusChanged(bool isStart)
{
}

void MeetingChatEventListener::onFileSendStart(ISDKFileSender* sender)
{
}

void MeetingChatEventListener::onFileReceived(ISDKFileReceiver* receiver)
{
}

void MeetingChatEventListener::onFileTransferProgress(SDKFileTransferInfo* info)
{
}
