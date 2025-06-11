#include "ClosedCaptionControllerEventListener.h"
#include <iostream>
#include <windows.h>

using namespace std;
using namespace ZOOMSDK;


ClosedCaptionControllerEventListener::ClosedCaptionControllerEventListener(void (*onLiveTransMsgInfoRec)(), void (*onClosedCaptionMsgRec)()) {
	onLiveTransMsgInfoRec_ = onLiveTransMsgInfoRec;
	onClosedCaptionMsgRec_ = onClosedCaptionMsgRec;
}

void ClosedCaptionControllerEventListener::onAssignedToSendCC(bool bAssigned) {}

void ClosedCaptionControllerEventListener::onClosedCaptionMsgReceived(const zchar_t* ccMsg, unsigned int sender_id, time_t time) {
	
	onClosedCaptionMsgRec_();
	
	std::wstring wstr(ccMsg);
	std::string utf8Str = wstringToUtf8(wstr);

	std::cout << "onClosedCaptionMsgReceived :" << utf8Str << endl;
}

void ClosedCaptionControllerEventListener::onLiveTranscriptionStatus(SDKLiveTranscriptionStatus status) {
	if (status == 1) {
		std::cout << "onLiveTranscriptionStatus :" << "SDK_LiveTranscription_Status_Start" << endl;
	}
	else if (status == 2) {
		std::cout << "onLiveTranscriptionStatus :" << "SDK_LiveTranscription_Status_User_Sub" << endl;
	}
	else if (status == 0) {
		std::cout << "onLiveTranscriptionStatus :" << "SDK_LiveTranscription_Status_Stop" << endl;
	}
	else if (status == 10) {
		std::cout << "onLiveTranscriptionStatus :" << "SDK_LiveTranscription_Status_Connecting" << endl;
	}
		
}

//deprecated
void ClosedCaptionControllerEventListener::onLiveTranscriptionMsgReceived(const zchar_t* ltMsg, unsigned int speaker_id, SDKLiveTranscriptionOperationType type) {

	/*if (type == SDK_LiveTranscription_OperationType_Complete) {

		std::wstring wstr(ltMsg);
		std::string utf8Str = wstringToUtf8(wstr);

		std::cout << "onLiveTranscriptionMsgReceived :" << utf8Str << endl;
	}*/
}

void ClosedCaptionControllerEventListener::onOriginalLanguageMsgReceived(ILiveTranscriptionMessageInfo* messageInfo) {
	if (messageInfo->GetMessageOperationType() == SDK_LiveTranscription_OperationType_Complete) {


		std::wstring wstr(messageInfo->GetMessageContent());
		std::string utf8Str = wstringToUtf8(wstr);

		std::cout << "onOriginalLanguageMsgReceived :" << utf8Str << endl;
	}
}

void ClosedCaptionControllerEventListener::onLiveTranscriptionMsgInfoReceived(ILiveTranscriptionMessageInfo* messageInfo) {

	if (messageInfo->GetMessageOperationType() == SDK_LiveTranscription_OperationType_Complete) {
		onLiveTransMsgInfoRec_();
	

		std::wstring wstr(messageInfo->GetMessageContent());
		std::string utf8Str = wstringToUtf8(wstr); 

		std::cout << "onLiveTranscriptionMsgInfoReceived :" << utf8Str << endl;
	

	}
}

void ClosedCaptionControllerEventListener::onLiveTranscriptionMsgError(ILiveTranscriptionLanguage* spokenLanguage, ILiveTranscriptionLanguage* transcriptLanguage) {
	std::cout << "onLiveTranscriptionMsgError" << endl;

}

void ClosedCaptionControllerEventListener::onRequestForLiveTranscriptReceived(unsigned int requester_id, bool bAnonymous) {}

void ClosedCaptionControllerEventListener::onRequestLiveTranscriptionStatusChange(bool bEnabled) {}

void ClosedCaptionControllerEventListener::onCaptionStatusChanged(bool bEnabled) {
	std::cout << "onCaptionStatusChanged :" << bEnabled << endl;
}

void ClosedCaptionControllerEventListener::onStartCaptionsRequestReceived(ICCRequestHandler* handler)
{
}

void ClosedCaptionControllerEventListener::onStartCaptionsRequestApproved()
{
}


void ClosedCaptionControllerEventListener::onManualCaptionStatusChanged(bool bEnabled)
{
}

std::string ClosedCaptionControllerEventListener::wstringToUtf8(const std::wstring& wstr) {
	std::string utf8Str;
	for (wchar_t wchar : wstr) {
		if (wchar < 0x80) {
			utf8Str += static_cast<char>(wchar);
		}
		else if (wchar < 0x800) {
			utf8Str += static_cast<char>(0xC0 | ((wchar >> 6) & 0x1F));
			utf8Str += static_cast<char>(0x80 | (wchar & 0x3F));
		}
		else {
			utf8Str += static_cast<char>(0xE0 | ((wchar >> 12) & 0x0F));
			utf8Str += static_cast<char>(0x80 | ((wchar >> 6) & 0x3F));
			utf8Str += static_cast<char>(0x80 | (wchar & 0x3F));
		}
	}
	return utf8Str;
}