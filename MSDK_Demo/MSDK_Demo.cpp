// This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <zoom_sdk.h>
#include <meeting_service_interface.h>
#include <rawdata/zoom_rawdata_api.h>
#include <rawdata/rawdata_video_source_helper_interface.h>
#include <setting_service_interface.h>
#include "network_connection_handler_interface.h"
#include "AuthServiceEventListener.h"
#include "NetworkConnectionHandler.h"
#include "MeetingServiceEventListener.h"
#include <fstream>
#include "json\json.h"
#include <sstream>

#include <thread>
#include <chrono>
#include "WebService.h"


//GetVideoRawData, GetAudioRawData, GetShareRawData
#include "MeetingRecordingCtrlEventListener.h"
#include "MeetingParticipantsCtrlEventListener.h"
#include <meeting_service_components/meeting_recording_interface.h>
#include <meeting_service_components/meeting_participants_ctrl_interface.h>

//GetAudioRawData
#include "ZoomSDKAudioRawDataDelegate.h"

//GetVideoRawData, GetShareRawData
#include "ZoomSDKRendererDelegate.h"


//SendAudioRawData
#include "ZoomSDKVirtualAudioMicEvent.h"

//SendVideoRawData
#include "ZoomSDKVideoSource.h"

//SendShareScreenRawData
#include "ZoomSDKShareSource.h"
#include <meeting_service_components/meeting_sharing_interface.h>

using namespace std;
using namespace Json;
using namespace ZOOMSDK;

bool g_exit = false;
IMeetingService* meetingService;
IAuthService* authService;
INetworkConnectionHelper* network_connection_helper;




wstring sdk_jwt;
UINT64 meeting_number;
wstring passcode;
constexpr auto CONFIG_FILE = "config.json";

//GetAudioRawData, SendAudioRawData
IZoomSDKAudioRawDataHelper* audioHelper;

//GetAudioRawData, SendAudioRawData, GetShareRawData
IMeetingRecordingController* m_pRecordController;
IMeetingParticipantsController* m_pParticipantsController;

//GetAudioRawData
ZoomSDKAudioRawDataDelegate* audio_source = new ZoomSDKAudioRawDataDelegate();
ISettingService* settingService;

//GetVideoRawData, GetShareRawData
ZoomSDKRendererDelegate* videoSource = new ZoomSDKRendererDelegate();
IZoomSDKRenderer* videoHelper;

//SendShareScreenRawData
ZoomSDKShareSource* virtual_share_source;
string video_share_source = "";
constexpr auto DEFAULT_VIDEO_SHARE_SOURCE = "Big_Buck_Bunny_1080_10s_1MB.mp4";


//SendVideoRawData
ZoomSDKVideoSource* virtual_camera_video_source;
string video_source = "";
constexpr auto DEFAULT_VIDEO_SOURCE = "Big_Buck_Bunny_1080_10s_1MB.mp4";

//SendAudioRawData
ZoomSDKVirtualAudioMicEvent* virtual_audio_source = new ZoomSDKVirtualAudioMicEvent();



//control variables
bool isJWTWebService = true;
bool SendVideoRawData = false;
bool SendAudioRawData = false;
bool SendShareScreenRawData = false;
bool GetVideoRawData = false;
bool GetAudioRawData = false;
bool GetShareRawData = false;

//GetVideoRawData
uint32_t getUserID() {
	m_pParticipantsController = meetingService->GetMeetingParticipantsController();
	int returnvalue = m_pParticipantsController->GetParticipantsList()->GetItem(0);


	return returnvalue;
}
//GetShareRawData
uint32_t getUserIDOfUserSharing() {


	IMeetingShareController* sharecontroller = meetingService->GetMeetingShareController();
	int returnvalue = sharecontroller->GetViewableShareSourceList()->GetItem(0);
	return returnvalue;
}


//GetVideoRawData, GetAudioRawData
void attemptToStartRawRecording() {

	m_pRecordController = meetingService->GetMeetingRecordingController();

	SDKError err1 = m_pRecordController->StartRawRecording();
	if (err1 != SDKERR_SUCCESS) {
		std::cout << "Error occurred";
	}
	if (GetShareRawData) {
		SDKError err = createRenderer(&videoHelper, videoSource);
		if (err != SDKERR_SUCCESS) {
			std::cout << "Error occurred";
			//handle error
		}
		else {
			videoHelper->subscribe(getUserIDOfUserSharing(), RAW_DATA_TYPE_SHARE);
		}

	}
	if (GetVideoRawData) {
		SDKError err = createRenderer(&videoHelper, videoSource);
		if (err != SDKERR_SUCCESS) {
			std::cout << "Error occurred";
			//handle error
		}
		else {
			videoHelper->setRawDataResolution(ZoomSDKResolution_720P);
			videoHelper->subscribe(getUserID(), RAW_DATA_TYPE_VIDEO);
		}
	}
	if (GetAudioRawData) {
		meetingService->GetMeetingAudioController()->JoinVoip();
		


		audioHelper = GetAudioRawdataHelper();
		if (audioHelper == NULL) {
			std::cout << "Error occurred";
			//handle error
		}
		SDKError err = audioHelper->subscribe(audio_source);
		if (err != SDKERR_SUCCESS) {
			std::cout << "Error occurred";
			//handle error
		}
	}
}

//GetAudioRawData, GetVideoRawData, GetShareRawData
bool CanIStartLocalRecording()
{

	IMeetingRecordingController* m_pRecordController = meetingService->GetMeetingRecordingController();
	if (m_pRecordController)
	{
		SDKError err = m_pRecordController->CanStartRecording(false, 0); //0 refers to current user
		if (err != SDKERR_SUCCESS) {
			std::cout << "Cannot start local recording...\n";
			std::cout << "Requesting...\n";
			m_pRecordController->RequestLocalRecordingPrivilege();
			//handle error
			return false;
		}
		else {
			std::cout << "Can start local recording...\n";
			return true;
		}

	}
}

//SendShareScreenRawData
void attemptToStartSendingShareScreenRaw() {

	virtual_share_source = new ZoomSDKShareSource(video_source);
	IZoomSDKShareSourceHelper* pShareSourceHelper = GetRawdataShareSourceHelper();
	if (pShareSourceHelper)
	{
		SDKError err = pShareSourceHelper->setExternalShareSource(virtual_share_source);
		if (err != SDKERR_SUCCESS) {

			printf("Error occurred:  $s\n", err);
			//handle error
		}
		else {
			printf("successfully set virtual share source...\n");
		}
	}

}
//SendShareScreenRawData
bool CanIStartSharing() {

	IMeetingShareController* m_pShareController = meetingService->GetMeetingShareController();

	return m_pShareController->CanStartShare();
}



//SendAudioRawData
//dreamtcs TODO fix this, audio only sends when unmuting
void attemptToStartRawAudioSending() {


	audioHelper = GetAudioRawdataHelper();
	if (audioHelper == NULL) {
		std::cout << "Error occurred";
		//handle error
	}

	SDKError err = audioHelper->setExternalAudioSource(virtual_audio_source);

	if (err != SDKERR_SUCCESS) {
		std::cout << "Error occurred";
		//handle error
	}

}

//SendVideoRawData
//dreamtcs TODO, video only start sending when video is "turned on" or "unmuted"
void attemptToStartRawVideoSending() {


	virtual_camera_video_source = new ZoomSDKVideoSource(video_source);
	IZoomSDKVideoSourceHelper* p_videoSourceHelper = GetRawdataVideoSourceHelper();

	if (p_videoSourceHelper) {
		SDKError err = p_videoSourceHelper->setExternalVideoSource(virtual_camera_video_source);
		if (err != SDKERR_SUCCESS) {
			printf("attemptToStartRawVideoSending(): Failed to set external video source, error code: %d\n", err);
		}
		else {

		}
	}
	else {
		printf("attemptToStartRawVideoSending(): Failed to get video source helper\n");
	}

}

void ShowErrorAndExit(SDKError err) {
	g_exit = true;
	string message = "";
	switch (err) {
	case SDKERR_INTERNAL_ERROR:
		message = ", check your SDK JWT.";
		break;
	case SDKERR_INVALID_PARAMETER:
		message = ", invalid parametersk, check your Meeting Number.";
	}
	printf("SDK Error: %d%s\n", err, message.c_str());
};


void onInMeeting() {


	printf("onInMeeting Invoked\n");

	//double check if you are in a meeting
	if (meetingService->GetMeetingStatus() == ZOOM_SDK_NAMESPACE::MEETING_STATUS_INMEETING) {
		printf("In Meeting Now...\n");

		if (GetVideoRawData || GetAudioRawData || GetShareRawData) {
			if (CanIStartLocalRecording()) {
				attemptToStartRawRecording();
			}
		}

		if (SendAudioRawData) {
			attemptToStartRawAudioSending();
		}

		if (SendVideoRawData) {
			attemptToStartRawVideoSending();
		}

		if (SendShareScreenRawData) {
			if (CanIStartSharing() == true) {
				//if  conditions above are true, start 
				attemptToStartSendingShareScreenRaw();
			}
		}

	}

}

void onMeetingEndsQuitApp() {
	g_exit = true;
}

void onMeetingJoined() {


}

void onIsHost() {
	//GetAudioRawData, GetVideoRawData, GetShareRawData
	if (CanIStartLocalRecording()) {
		printf("Is host now...\n");
		attemptToStartRawRecording();
	}
}

void onIsCoHost() {
	//GetAudioRawData, GetVideoRawData, GetShareRawData
	if (CanIStartLocalRecording()) {
		printf("Is co-host now...\n");
		attemptToStartRawRecording();
	}
}
void onIsGivenRecordingPermission() {
	//GetAudioRawData, GetVideoRawData, GetShareRawData
	if (CanIStartLocalRecording()) {
		printf("Is given recording permissions now...\n");
		attemptToStartRawRecording();
	}
}


wstring StringToWString(string input)
{
	wstring output;
	output.assign(input.begin(), input.end());
	return output;
}

string WStringToString(wstring input)
{
	string output;
	output.assign(input.begin(), input.end());
	return output;
}

wstring QuestionInput(string qustion)
{
	wstring input;
	std::cout << qustion;
	getline(wcin, input);
	return input;
}
/// <summary>
/// Loads configuration from config.json file.
/// This config file contains the JWT token, meeting number, passcode and mp4 file for streaming
/// </summary>
void LoadConfig() {
	ifstream f(CONFIG_FILE);
	Value config;
	printf("Trying to find meeting parameters from config.json.\n");
	bool isConfigFileOpened = f.is_open();
	if (isConfigFileOpened) {
		try {
			f >> config;
		}
		catch (exception) {
			printf("config.json is in wrong format.\n");
		}
	}
	else {
		printf("Didn't find config.json file.\n");
	}
	if (!isConfigFileOpened || config["sdk_jwt"].empty() || config["sdk_jwt"].asString() == "") {
		sdk_jwt = QuestionInput("SDK JWT: ");
	}
	else {
		sdk_jwt = StringToWString(config["sdk_jwt"].asString());
		printf("Found \"SDK JWT\" from %s: \n\"%s\"\n", CONFIG_FILE, WStringToString(sdk_jwt).c_str());
	}
	bool toQuestionForMeetingNumber = false;
	if (!isConfigFileOpened || config["meeting_number"].empty() || config["meeting_number"].asString() == "")
		toQuestionForMeetingNumber = true;
	else {
		try {
			meeting_number = config["meeting_number"].asUInt64();
			std::ostringstream oss;
			oss << meeting_number;
			printf("Found \"Meeting Number\" from %s: \"%s\"\n", CONFIG_FILE, string(oss.str()).c_str());
		}
		catch (exception)
		{
			try {
				string value = config["meeting_number"].asString();
				meeting_number = stoull(value, nullptr, 10);
				std::ostringstream oss;
				oss << meeting_number;
				printf("Found \"Meeting Number\" from %s: \"%s\"\n", CONFIG_FILE, string(oss.str()).c_str());
			}
			catch (exception)
			{
				printf("Failed to read \"meeting_number\" from config.json, it should include only numbers.\n");
				toQuestionForMeetingNumber = true;
			}
		}
	}
	while (toQuestionForMeetingNumber) {
		std::wcout << "Meeting Number: ";
		string input;
		getline(cin, input);
		try {
			meeting_number = stoull(input, nullptr, 10);
			toQuestionForMeetingNumber = false;
		}
		catch (exception)
		{
			printf("Meeting Number should include numbers.\n");
		}
	}
	if (!isConfigFileOpened || config["passcode"].empty() || config["passcode"].asString() == "") {
		passcode = QuestionInput("Passcode: ");
	}
	else {
		passcode = StringToWString(config["passcode"].asString());
		printf("Found \"Passcode\" from %s: \"%s\"\n", CONFIG_FILE, WStringToString(passcode).c_str());
	}
	if (!isConfigFileOpened || config["video_source"].empty() || config["video_source"].asString() == "") {
		video_source = WStringToString(QuestionInput("Video Source (file path or URL): "));
	}
	else {
		video_source = config["video_source"].asString();
		printf("Found \"Video Source\" from %s: \"%s\"\n", CONFIG_FILE, video_source.c_str());
	}
	if (video_source == "") {
		video_source = DEFAULT_VIDEO_SOURCE;
		printf("No video source provided, use the default video source: %s.\n", video_source.c_str());
	}


	if (!isConfigFileOpened || config["video_share_source"].empty() || config["video_share_source"].asString() == "") {
		video_share_source = WStringToString(QuestionInput("Video Share Source (file path or URL): "));
	}
	else {
		video_share_source = config["video_share_source"].asString();
		printf("Found \"Video Share Source\" from %s: \"%s\"\n", CONFIG_FILE, video_share_source.c_str());
	}
	if (video_share_source == "") {
		video_share_source = DEFAULT_VIDEO_SHARE_SOURCE;
		printf("No video share source provided, use the default video share source: %s.\n", video_share_source.c_str());
	}

}

/// <summary>
/// Join a zoom meeting
/// </summary>
void JoinMeeting()
{
	SDKError err(SDKError::SDKERR_SUCCESS);


	//try to create the meetingservice object, this object will be used to join the meeting
	if ((err = CreateMeetingService(&meetingService)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "MeetingService created." << std::endl;

	//GetVideoRawData, GetAudioRawData, GetShareRawData
	m_pParticipantsController = meetingService->GetMeetingParticipantsController();

	// try to create the meeting configuration object, this object will be used to configure the meeting
	// joinMeetingWithoutLogin Parameters will join a meeting as a guest user, who typically don't sign-in / login.
	JoinParam joinMeetingParam;
	JoinParam4WithoutLogin joinMeetingWithoutLoginParam;
	joinMeetingParam.userType = SDK_UT_WITHOUT_LOGIN;
	joinMeetingWithoutLoginParam.meetingNumber = meeting_number;
	joinMeetingWithoutLoginParam.psw = passcode.c_str();
	joinMeetingWithoutLoginParam.userName = L"RawDataSender(VirtualCam)";
	joinMeetingWithoutLoginParam.userZAK = L"";
	joinMeetingWithoutLoginParam.join_token = NULL;
	joinMeetingWithoutLoginParam.vanityID = NULL;
	joinMeetingWithoutLoginParam.customer_key = NULL;
	joinMeetingWithoutLoginParam.webinarToken = NULL;
	joinMeetingWithoutLoginParam.app_privilege_token = NULL;
	joinMeetingWithoutLoginParam.hDirectShareAppWnd = NULL;
	joinMeetingWithoutLoginParam.isAudioOff = true;
	joinMeetingWithoutLoginParam.isVideoOff = true;
	joinMeetingWithoutLoginParam.isDirectShareDesktop = false;
	joinMeetingParam.param.withoutloginuserJoin = joinMeetingWithoutLoginParam;
	if (GetAudioRawData) {
		joinMeetingWithoutLoginParam.isAudioOff = false;
	}


	// Set the event listener for meeting joining status
	meetingService->SetEvent(new MeetingServiceEventListener(&onMeetingJoined, &onMeetingEndsQuitApp, &onInMeeting));

	//GetAudioRawData, GetVideoRawData, GetShareRawData
	// Set the event listener for host/cohost given to bot
	m_pParticipantsController->SetEvent(new MeetingParticipantsCtrlEventListener(&onIsHost, &onIsCoHost, &onIsGivenRecordingPermission));

	//GetAudioRawData, GetVideoRawData, GetShareRawData
	// Set the event listener for recording permission given to bot
	m_pRecordController = meetingService->GetMeetingRecordingController();
	m_pRecordController->SetEvent(new MeetingRecordingCtrlEventListener(&onIsGivenRecordingPermission));


	if (GetAudioRawData) {
		//Create the setting service, and set join audio to true
		CreateSettingService(&settingService);
		std::cerr << "Settingservice created." << std::endl;
		ZOOM_SDK_NAMESPACE::IAudioSettingContext* pAudioContext = settingService->GetAudioSettings();
		if (pAudioContext)
		{
			pAudioContext->EnableAutoJoinAudio(true);
		}
	}

	//join meeting
	if ((err = meetingService->Join(joinMeetingParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	else std::cout << "Joining Meeting..." << std::endl;


}
/// <summary>
/// Authorize SDK with JWT Token
/// </summary>
void SDKAuth()
{
	SDKError err(SDKError::SDKERR_SUCCESS);

	if ((err = CreateAuthService(&authService)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "AuthService created." << std::endl;
	AuthContext authContext;
	if ((err = authService->SetEvent(new AuthServiceEventListener(JoinMeeting))) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "AuthServiceEventListener added." << std::endl;
	//authContext.jwt_token = sdk_jwt.c_str();
	if (isJWTWebService) {
		authContext.jwt_token = GetSignatureFromWebService();
	}
	else {
		authContext.jwt_token = sdk_jwt.c_str();
	}
	if ((err = authService->SDKAuth(authContext)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	else std::cout << "Auth call started, auth in progress." << std::endl;
}

/// <summary>
/// Initialize SDK with JWT Token
/// </summary>
void InitSDK()
{
	SDKError err(SDKError::SDKERR_SUCCESS);

	InitParam initParam;
	initParam.strWebDomain = L"https://zoom.us";
	initParam.enableLogByDefault = true;
	if ((err = InitSDK(initParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "SDK Initialized." << std::endl;
	if ((err = CreateNetworkConnectionHelper(&network_connection_helper)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "CreateNetworkConnectionHelper created." << std::endl;
	if ((err = network_connection_helper->RegisterNetworkConnectionHandler(new NetworkConnectionHandler(&SDKAuth))) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "NetworkConnectionHandler registered. Detecting proxy." << std::endl;
}
void cleanup() {
	if (meetingService) DestroyMeetingService(meetingService);
	if (authService) DestroyAuthService(authService);
	if (network_connection_helper) DestroyNetworkConnectionHelper(network_connection_helper);
	CleanUPSDK(); // must do this, or it will crash. 
}
/// <summary>
/// main method for entry point
/// </summary>
/// <returns></returns>
int main()
{
	LoadConfig();
	InitSDK();

	int bRet = false;
	MSG msg;
	while (!g_exit && (bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	cleanup();
	return 0;
}

