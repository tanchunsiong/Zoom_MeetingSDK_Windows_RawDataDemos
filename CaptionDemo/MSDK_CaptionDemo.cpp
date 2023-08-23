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
#include "MeetingParticipantsCtrlEventListener.h"
#include <fstream>
#include "json\json.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <meeting_service_components/meeting_participants_ctrl_interface.h>
#include <meeting_service_components/meeting_sharing_interface.h>
#include <list>
#include "WebService.h"

#include <meeting_service_components/meeting_closedcaption_interface.h> //CaptionDemo
#include "ClosedCaptionControllerEventListener.h" //CaptionDemo
#include "MSDK_CaptionDemo.h"

using namespace std;
using namespace Json;
using namespace ZOOMSDK;

bool g_exit = false;

//CaptionDemo

IMeetingService* meetingService;
IClosedCaptionController* captionController; //CaptionDemo
ClosedCaptionControllerEventListener* captionEventListener;  //CaptionDemo
IAuthService* authService;
INetworkConnectionHelper* network_connection_helper;
//references for send raw video data
wstring sdk_jwt;
UINT64 meeting_number;
wstring passcode;
string video_source = "";
constexpr auto DEFAULT_VIDEO_SOURCE = "Big_Buck_Bunny_1080_10s_1MB.mp4";
constexpr auto CONFIG_FILE = "config.json";
;

bool isJWTWebService = true;

bool isStartMeeting = false;


IMeetingParticipantsController* m_pParticipantsController;



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



	//CaptionDemo

	captionController = meetingService->GetMeetingClosedCaptionController();
	captionEventListener = new ClosedCaptionControllerEventListener(&onLiveTransMsgInfoRec, &onClosedCaptionMsgRec);
	SDKError setEventErr = captionController->SetEvent(captionEventListener);
	cout << "Error code for setting captionController event " << setEventErr << endl;


}

void onLiveTransMsgInfoRec()
{
	printf("onLiveTransMsgInfoRec...\n");
}

void onClosedCaptionMsgRec()
{
	printf("onClosedCaptionMsgRec...\n");

}

void onMeetingEndsQuitApp() {
	g_exit = true;
}

void onMeetingJoined() {

	printf("Joining Meeting...\n");

	//std::thread t1(prereqCheckForRawVideoSend);
	//t1.detach(); //run in different thread

}


void onIsHost() {

	//CaptionDemo
	if (m_pParticipantsController->GetMySelfUser()->IsHost()) {

		//closed caption demo
		if (false) {
			
			SDKError enablecaptionErr = captionController->EnableCaptions(true);
			//SDKError requestStartLiveTransErr = captionController->RequestToStartLiveTranscription(false);
			SDKError enablmanualecaptionErr = captionController->EnableMeetingManualCaption(true);
			SDKError enableassignccprivErr = captionController->AssignCCPriviledge(0, true);
			std::wstring cctoSend = L"hello world";
			SDKError sendcaptionErr = captionController->SendClosedCaption(cctoSend.c_str());
		}



		//transcription demo
		if (true) {

			//start live transcription
			SDKError sdkerr1 = captionController->StartLiveTranscription();
			//set my own spoken language, 0 is for english
			SDKError sdkerr2 = captionController->SetMeetingSpokenLanguage(0);
			//set translated language, 1 is for mandarin/chinese. -1 is to disable translation
			SDKError sdkerr3 = captionController->SetTranslationLanguage(1);
			
			SDKError sdkerr4 = captionController->EnableReceiveSpokenLanguageContent(true);
			std::wstring testString = L"hello world";

		}
	}

	//接收手动字幕的callback
	//virtual void onClosedCaptionMsgReceived(const wchar_t* ccMsg, unsigned int sender_id, time_t time) = 0;

}

void onIsCoHost() {

}
void onIsGivenRecordingPermission() {

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
	cout << qustion;
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
		wcout << "Meeting Number: ";
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

}

/// <summary>
/// Join a zoom meeting
/// </summary>
void JoinMeeting()
{
	SDKError err(SDKError::SDKERR_SUCCESS);


	//try to create the meetingservice object, this object will be used to join the meeting
	if ((err = CreateMeetingService(&meetingService)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	cout << "MeetingService created." << endl;

	m_pParticipantsController = meetingService->GetMeetingParticipantsController();

	//before joining a meeting, create the setting service
	ISettingService* settingservice;
	CreateSettingService(&settingservice);

	////CaptionDemo
	//captionController = meetingService->GetMeetingClosedCaptionController();
	//SDKError setEventErr = captionController->SetEvent(new ClosedCaptionControllerEventListener(&onLiveTransMsgInfoRec, &onClosedCaptionMsgRec));
	//cout << "Error code for setting captionController event " << setEventErr << endl;

	//use the audiosetting to set EnableAlwaysMuteMicWhenJoinVoip
	IAudioSettingContext* audioSetting;
	audioSetting = settingservice->GetAudioSettings();
	audioSetting->EnableAlwaysMuteMicWhenJoinVoip(true);

	if (!isStartMeeting) {




		JoinParam joinMeetingParam;
		JoinParam4WithoutLogin joinMeetingWithoutLoginParam;
		joinMeetingParam.userType = SDK_UT_WITHOUT_LOGIN;
		joinMeetingWithoutLoginParam.meetingNumber = meeting_number;
		joinMeetingWithoutLoginParam.psw = passcode.c_str();;
		joinMeetingWithoutLoginParam.userName = L"RawDataSender(VirtualCam)";
		joinMeetingWithoutLoginParam.userZAK = L"";
		//joinMeetingWithoutLoginParam.userZAK = L"yJ0eXAiOiJKV1QiLCJzdiI6IjAwMDAwMSIsInptX3NrbSI6InptX28ybSIsImFsZyI6IkhTMjU2In0.eyJhdWQiOiJjbGllbnRzbSIsInVpZCI6Ii1HQlJ0bHFJUTF5WkRsYkEzMnhuWXciLCJpc3MiOiJ3ZWIiLCJzayI6IjAiLCJzdHkiOjk5LCJ3Y2QiOiJ1czA1IiwiY2x0IjowLCJleHAiOjE2OTIyNTM0ODAsImlhdCI6MTY5MjI0NjI4MCwiYWlkIjoiOXNTX1BCMnlSODI3bm10c3NnOXdnUSIsImNpZCI6IiJ9.X9Ika89y3RY6pSL-j48tu6LzHD9zZTHNkafbDZ1ZzG0";
		//joinMeetingWithoutLoginParam.app_privilege_token = L"lr6qgktey";
		joinMeetingWithoutLoginParam.join_token = NULL;
		joinMeetingWithoutLoginParam.vanityID = NULL;
		//joinMeetingWithoutLoginParam.vanityID = L"magaoay";
		joinMeetingWithoutLoginParam.customer_key = L"abcdefghijklmnopqrtsuvwxyz1234567890";
		joinMeetingWithoutLoginParam.webinarToken = NULL;
		joinMeetingWithoutLoginParam.app_privilege_token = NULL;
		joinMeetingWithoutLoginParam.hDirectShareAppWnd = NULL;
		joinMeetingWithoutLoginParam.isAudioOff = true;
		joinMeetingWithoutLoginParam.isVideoOff = true;
		joinMeetingWithoutLoginParam.isDirectShareDesktop = false;
		joinMeetingParam.param.withoutloginuserJoin = joinMeetingWithoutLoginParam;

		// Set the event listener
		meetingService->SetEvent(new MeetingServiceEventListener(&onMeetingJoined, &onMeetingEndsQuitApp, &onInMeeting));
		m_pParticipantsController->SetEvent(new MeetingParticipantsCtrlEventListener(&onIsHost, &onIsCoHost, &onIsGivenRecordingPermission));


		//join meeting
		if ((err = meetingService->Join(joinMeetingParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
		else cout << "Joining Meeting..." << endl;

	}
	//isStartMeeting
	else {



		ZOOM_SDK_NAMESPACE::StartParam startMeetingParam;
		startMeetingParam.userType = ZOOM_SDK_NAMESPACE::SDK_UT_WITHOUT_LOGIN;
		StartParam4WithoutLogin startMeetingWithoutLoginParam = startMeetingParam.param.withoutloginStart;
		startMeetingParam.userType = ZOOM_SDK_NAMESPACE::SDK_UT_WITHOUT_LOGIN;

		//startMeetingWithoutLoginParam.meetingNumber = 2096270835;
		startMeetingWithoutLoginParam.meetingNumber = meeting_number;
		startMeetingWithoutLoginParam.zoomuserType = ZoomUserType_APIUSER;
		//startMeetingWithoutLoginParam.vanityID = L"magaoay";
		startMeetingWithoutLoginParam.userName = L"RawDataSender(VirtualCam)";
		startMeetingWithoutLoginParam.userZAK = L"eyJ0eXAiOiJKV1QiLCJzdiI6IjAwMDAwMSIsInptX3NrbSI6InptX28ybSIsImFsZyI6IkhTMjU2In0.eyJhdWQiOiJjbGllbnRzbSIsInVpZCI6IktvMGtIb2tUU2t1NXpTVy1GU2RIMEEiLCJpc3MiOiJ3ZWIiLCJzayI6IjYyOTkzMTI0OTc4MDkwNjIwNTkiLCJzdHkiOjEwMCwid2NkIjoiYXcxIiwiY2x0IjowLCJleHAiOjE2ODk4Mzk4NzAsImlhdCI6MTY4OTgzMjY3MCwiYWlkIjoiMnlkN0JNbFRSR3VHbktyazd2QXpmUSIsImNpZCI6IiJ9.-y7lRG5o3sDt9xgQm_uaW1dZL1wOC8Pu2IS0YBV5x54";
		//startMeetingWithoutLoginParam.join_token = NULL;
		startMeetingWithoutLoginParam.customer_key = NULL;
		//startMeetingWithoutLoginParam.webinarToken = NULL;
		//startMeetingWithoutLoginParam.app_privilege_token = NULL;
		startMeetingWithoutLoginParam.hDirectShareAppWnd = NULL;
		startMeetingWithoutLoginParam.isAudioOff = true;
		startMeetingWithoutLoginParam.isVideoOff = true;
		startMeetingWithoutLoginParam.isDirectShareDesktop = false;

		startMeetingParam.param.withoutloginStart = startMeetingWithoutLoginParam;


		//start meeting
		if ((err = meetingService->Start(startMeetingParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
		else cout << "Joining Meeting..." << endl;
	}
}
/// <summary>
/// Authorize SDK with JWT Token
/// </summary>
void SDKAuth()
{
	SDKError err(SDKError::SDKERR_SUCCESS);

	if ((err = CreateAuthService(&authService)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	cout << "AuthService created." << endl;
	AuthContext authContext;
	if ((err = authService->SetEvent(new AuthServiceEventListener(JoinMeeting))) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	cout << "AuthServiceEventListener added." << endl;
	//authContext.jwt_token = sdk_jwt.c_str();

	//isJWTWebService
	if (isJWTWebService) {
		authContext.jwt_token = GetSignatureFromWebService();


	}
	else {
		authContext.jwt_token = sdk_jwt.c_str();
	}

	if ((err = authService->SDKAuth(authContext)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	else cout << "Auth call started, auth in progress." << endl;

}

/// <summary>
/// Initialize SDK with JWT Token
/// </summary>
void InitSDK()
{
	SDKError err(SDKError::SDKERR_SUCCESS);

	InitParam initParam;
	//initParam.strWebDomain = L"https://dev-integration.zoomdev.us/";
	initParam.strWebDomain = L"https://zoom.us/";
	initParam.enableLogByDefault = true;
	if ((err = InitSDK(initParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	cout << "SDK Initialized." << endl;
	if ((err = CreateNetworkConnectionHelper(&network_connection_helper)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	cout << "CreateNetworkConnectionHelper created." << endl;
	if ((err = network_connection_helper->RegisterNetworkConnectionHandler(new NetworkConnectionHandler(&SDKAuth))) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	cout << "NetworkConnectionHandler registered. Detecting proxy." << endl;
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
	if (meetingService) DestroyMeetingService(meetingService);
	if (authService) DestroyAuthService(authService);
	if (network_connection_helper) DestroyNetworkConnectionHelper(network_connection_helper);
	CleanUPSDK(); // must do this, or it will crash. 
}

