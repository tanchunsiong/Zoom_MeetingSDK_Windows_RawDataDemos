#include "ClosedCaptionControllerEventListener.h"
#include <iostream>

using namespace std;
using namespace ZOOMSDK;


ClosedCaptionControllerEventListener::ClosedCaptionControllerEventListener(void (*onLiveTransMsgInfoRec)(), void (*onClosedCaptionMsgRec)()) {
	onLiveTransMsgInfoRec_ = onLiveTransMsgInfoRec;
	onClosedCaptionMsgRec_ = onClosedCaptionMsgRec;
}

void ClosedCaptionControllerEventListener::onAssignedToSendCC(bool bAssigned) {}

 void ClosedCaptionControllerEventListener::onClosedCaptionMsgReceived(const zchar_t* ccMsg, unsigned int sender_id, time_t time) {
	 onClosedCaptionMsgRec_();
	 cout << sender_id << endl;
	 cout << ccMsg << endl;
	
 }

 void ClosedCaptionControllerEventListener::onLiveTranscriptionStatus(SDKLiveTranscriptionStatus status) {}

 void ClosedCaptionControllerEventListener::onLiveTranscriptionMsgReceived(const zchar_t* ltMsg, unsigned int speaker_id, SDKLiveTranscriptionOperationType type) {}

 //deprecated
 void ClosedCaptionControllerEventListener::onOriginalLanguageMsgReceived(ILiveTranscriptionMessageInfo* messageInfo) {}

 void ClosedCaptionControllerEventListener::onLiveTranscriptionMsgInfoReceived(ILiveTranscriptionMessageInfo* messageInfo) {
	 onLiveTransMsgInfoRec_();
	 
	 cout << messageInfo->GetSpeakerName() << endl;
	 cout << messageInfo->GetMessageContent() << endl;
 }

 void ClosedCaptionControllerEventListener::onLiveTranscriptionMsgError(ILiveTranscriptionLanguage* spokenLanguage, ILiveTranscriptionLanguage* transcriptLanguage) {}

 void ClosedCaptionControllerEventListener::onRequestForLiveTranscriptReceived(unsigned int requester_id, bool bAnonymous) {}

 void ClosedCaptionControllerEventListener::onRequestLiveTranscriptionStatusChange(bool bEnabled) {}

 void ClosedCaptionControllerEventListener::onCaptionStatusChanged(bool bEnabled) {
	 cout << "onCaptionStatusChanged" << endl;
 }