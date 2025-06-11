#pragma once
#include <windows.h>
#include "zoom_sdk.h"
#include <meeting_service_components/meeting_closedcaption_interface.h> //CaptionDemo
#include <string>

using namespace std;
using namespace ZOOMSDK;


class ClosedCaptionControllerEventListener :
	public IClosedCaptionControllerEvent
{
	void (*onLiveTransMsgInfoRec_)();
	void (*onClosedCaptionMsgRec_)();

public:
	ClosedCaptionControllerEventListener(void (*onLiveTransMsgInfoRec_)(), void (*onClosedCaptionMsgRec_)());


	/// \brief Callback event when the user is assigned to send Closed Caption messages.
	virtual void onAssignedToSendCC(bool bAssigned);

	/// \brief Callback event when the user receives Closed Caption messages.
	/// \If the meeting supports multi-language transcription and the host sets manual captions to true,
	/// \attendees must set the translation language id to -1 to receive closed captioned messages.
	/// \param ccMsg: an object pointer to the closed caption message content. 
	/// \param sender_id: the sender id of a closed caption message. 
	/// \param time: the time when a caption message was sent.
	virtual void onClosedCaptionMsgReceived(const zchar_t* ccMsg, unsigned int sender_id, time_t time);

	/// \brief live transcription status changed callback.
	/// \param status: the live transcription status. For more details, see \link SDKLiveTranscriptionStatus \endlink.
	virtual void onLiveTranscriptionStatus(SDKLiveTranscriptionStatus status);

	/// \brief live transcription message received callback.
	/// \param ltMsg: an object pointer to the live transcription message content. 
	/// \param ltSpearkId: the speaker id of the live transcription message. 
	/// \param type: the live transcription operation type. For more details, see \link SDKLiveTranscriptionOperationType \endlink.
	/// \deprecated This interface will be marked as deprecated, then it will be instead by onLiveTranscriptionMsgInfoReceived, please stop using it.
	virtual void onLiveTranscriptionMsgReceived(const zchar_t* ltMsg, unsigned int speaker_id, SDKLiveTranscriptionOperationType type);

	/// \brief original language message received callback.
	/// \param messageInfo The spoken language message, see \link ILiveTranscriptionMessageInfo \endlink.
	virtual void onOriginalLanguageMsgReceived(ILiveTranscriptionMessageInfo* messageInfo);

	/// \brief live transcription message received callback.
	/// \param messageInfo The live transcription message, see \link ILiveTranscriptionMessageInfo \endlink.
	virtual void onLiveTranscriptionMsgInfoReceived(ILiveTranscriptionMessageInfo* messageInfo);

	/// \brief The translation message error callback.
	/// \param speakingLanguage: an object of the spoken message language. 
	/// \param transcriptLanguageId: an object of the message language you want to translate.
	virtual void onLiveTranscriptionMsgError(ILiveTranscriptionLanguage* spokenLanguage, ILiveTranscriptionLanguage* transcriptLanguage);

	/// \brief only host can receive this callback.
	/// \param requester_id: the request user id, if param bAnonymous is true, the requester_id is 0, no meanings. 
	/// \param bAnonymous: specify the request whether user anonymous request.
	virtual void onRequestForLiveTranscriptReceived(unsigned int requester_id, bool bAnonymous);

	/// \brief request live transcription status changed callback.
	/// \param bEnabled Specify the request live transcription status is enable, true enable, false disabled.
	virtual void onRequestLiveTranscriptionStatusChange(bool bEnabled);

	/// \brief Sink the event of captions enabled status changed.
	/// \param bEnabled True means the host enables the captions, otherwise means the host disables the captions.
	virtual void onCaptionStatusChanged(bool bEnabled);

	/// \brief Sink the event to start captions request.
	/// \param handler A pointer to the ICCRequestHandler. For more details, see \link ICCRequestHandler \endlink.
	virtual void onStartCaptionsRequestReceived(ICCRequestHandler* handler);

	/// \brief Sink the event to start captions request was approved.
	virtual void onStartCaptionsRequestApproved();


	/// \brief Sink the event of manual captions enablement status change.
/// \param bEnabled True means the host enables the manual captions. Otherwise the host disabled the manual captions.
	virtual void onManualCaptionStatusChanged(bool bEnabled);

	virtual std::string wstringToUtf8(const std::wstring& wstr);
};
