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
#include <meeting_service_components/meeting_audio_interface.h>
#include <meeting_service_components/meeting_participants_ctrl_interface.h>
#include <meeting_service_components/meeting_sharing_interface.h>
#include <list>
#include "WebService.h"
#include "MeetingChatEventListener.h" //chatdemo

#include <locale>
#include <codecvt>

using namespace std;
using namespace Json;
using namespace ZOOMSDK;

bool g_exit = false;
IMeetingService* meetingService;
IMeetingChatController* meetingchatcontroller; //chatdemo
MeetingChatEventListener* meetingchateventlistener;//chatdemo
IAuthService* authService;
INetworkConnectionHelper* network_connection_helper;
//references for send raw video data
wstring sdk_jwt;
UINT64 meeting_number;
wstring passcode;
wstring webservice_endpoint;
string video_source = "";
constexpr auto DEFAULT_VIDEO_SOURCE = "Big_Buck_Bunny_1080_10s_1MB.mp4";
constexpr auto CONFIG_FILE = "config.json";


bool isJWTWebService = true;

inline bool IsInMeeting(ZOOM_SDK_NAMESPACE::MeetingStatus status)
{
	bool bInMeeting(false);
	if (status == ZOOM_SDK_NAMESPACE::MEETING_STATUS_INMEETING)
	{

	}

	return bInMeeting;
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

	//chatdemo
	//receive text
	meetingchatcontroller = meetingService->GetMeetingChatController();
	meetingchateventlistener = new MeetingChatEventListener();
	meetingchatcontroller->SetEvent(meetingchateventlistener);


	//chatdemo
	//send text

	
	
	 

	std::wstring wstr = L"Hello, 世界!";
	wchar_t wcharArray[1024]; // Choose an appropriate size
	wcscpy_s(wcharArray, wstr.c_str());

	
	IChatMsgInfoBuilder* chatbuilder =  meetingchatcontroller->GetChatMessageBuilder();
	chatbuilder->SetReceiver(0);
	chatbuilder->SetMessageType(SDKChatMessageType_To_All);
	chatbuilder->SetContent(wcharArray);
	chatbuilder->Build();
	meetingchatcontroller->SendChatMsgTo(chatbuilder->Build());
}



void onMeetingEndsQuitApp() {
	g_exit = true;
}

void onMeetingJoined() {

	printf("Joining Meeting...\n");

	//std::thread t1(prereqCheckForRawVideoSend);
	//t1.detach(); //run in different thread

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
		catch (exception&) {
			printf("config.json is in wrong format.\n");
		}
	}
	else {
		printf("Didn't find config.json file.\n");
	}

	// SDK JWT
	if (!isConfigFileOpened || config["sdk_jwt"].empty() || config["sdk_jwt"].asString() == "") {
		sdk_jwt = QuestionInput("SDK JWT: ");
	}
	else {
		sdk_jwt = StringToWString(config["sdk_jwt"].asString());
		printf("Found \"SDK JWT\" from %s: \n\"%s\"\n", CONFIG_FILE, WStringToString(sdk_jwt).c_str());
	}

	// Meeting Number
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
		catch (exception&) {
			try {
				string value = config["meeting_number"].asString();
				meeting_number = stoull(value, nullptr, 10);
				std::ostringstream oss;
				oss << meeting_number;
				printf("Found \"Meeting Number\" from %s: \"%s\"\n", CONFIG_FILE, string(oss.str()).c_str());
			}
			catch (exception&) {
				printf("Failed to read \"meeting_number\" from config.json, it should include only numbers.\n");
				toQuestionForMeetingNumber = true;
			}
		}
	}
	while (toQuestionForMeetingNumber) {
		std::wcout << L"Meeting Number: ";
		string input;
		getline(cin, input);
		try {
			meeting_number = stoull(input, nullptr, 10);
			toQuestionForMeetingNumber = false;
		}
		catch (exception&) {
			printf("Meeting Number should include numbers.\n");
		}
	}

	// Passcode
	if (!isConfigFileOpened || config["passcode"].empty() || config["passcode"].asString() == "") {
		passcode = QuestionInput("Passcode: ");
	}
	else {
		passcode = StringToWString(config["passcode"].asString());
		printf("Found \"Passcode\" from %s: \"%s\"\n", CONFIG_FILE, WStringToString(passcode).c_str());
	}

	// Video Source
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

	// Webservice Endpoint (new)
	if (!isConfigFileOpened || config["webservice_endpoint"].empty() || config["webservice_endpoint"].asString() == "") {
		webservice_endpoint = QuestionInput("Webservice Endpoint (https URL): ");
	}
	else {
		webservice_endpoint = StringToWString(config["webservice_endpoint"].asString());
		printf("Found \"Webservice Endpoint\" from %s: \"%s\"\n", CONFIG_FILE, WStringToString(webservice_endpoint).c_str());
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





	// try to create the meeting configuration object, this object will be used to configure the meeting
	// joinMeetingWithoutLogin Parameters will join a meeting as a guest user, who typically don't sign-in / login.

	ZOOM_SDK_NAMESPACE::JoinParam joinMeetingParam;
	JoinParam4WithoutLogin joinMeetingWithoutLoginParam = joinMeetingParam.param.withoutloginuserJoin;
	joinMeetingParam.userType = SDK_UT_WITHOUT_LOGIN;
	joinMeetingWithoutLoginParam.meetingNumber = meeting_number;
	joinMeetingWithoutLoginParam.psw = passcode.c_str();
	wchar_t screenName[] = L"Chun";
	joinMeetingWithoutLoginParam.userName = screenName; 
	joinMeetingWithoutLoginParam.userZAK = L"";
	joinMeetingWithoutLoginParam.app_privilege_token = L"";
	//joinMeetingWithoutLoginParam.app_privilege_token = NULL;
	joinMeetingWithoutLoginParam.join_token = NULL;
	joinMeetingWithoutLoginParam.vanityID = NULL;
	joinMeetingWithoutLoginParam.customer_key = NULL;
	joinMeetingWithoutLoginParam.webinarToken = NULL;

	joinMeetingWithoutLoginParam.hDirectShareAppWnd = NULL;
	joinMeetingWithoutLoginParam.isAudioOff = true;
	joinMeetingWithoutLoginParam.isVideoOff = true;
	joinMeetingWithoutLoginParam.isDirectShareDesktop = false;
	joinMeetingParam.param.withoutloginuserJoin = joinMeetingWithoutLoginParam;
	// Set the event listener
	meetingService->SetEvent(new MeetingServiceEventListener(&onMeetingJoined, &onMeetingEndsQuitApp, &onInMeeting));

	//join meeting
	if ((err = meetingService->Join(joinMeetingParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	else std::cout << "Joining Meeting..." << std::endl;


	//ZOOM_SDK_NAMESPACE::StartParam startMeetingParam;
	//StartParam4WithoutLogin startMeetingWithoutLoginParam;
	//startMeetingParam.userType = ZOOM_SDK_NAMESPACE::SDK_UT_WITHOUT_LOGIN;
	//startMeetingWithoutLoginParam.meetingNumber = meeting_number;
	////startMeetingWithoutLoginParam.psw = passcode.c_str(); 
	//startMeetingWithoutLoginParam.zoomuserType = ZoomUserType_APIUSER;
	//startMeetingWithoutLoginParam.userID = L"tanchunsiong.sg@fakegmail.com";
	//startMeetingWithoutLoginParam.userName = L"RawDataSender(VirtualCam)";
	//startMeetingWithoutLoginParam.userZAK = L"eyJ0eXAiOiJKV1QiLCJzdiI6IjAwMDAwMSIsInptX3NrbSI6InptX28ybSIsImFsZyI6IkhTMjU2In0.eyJhdWQiOiJjbGllbnRzbSIsInVpZCI6Ijc0N0tEcXlFUnZ1Z3Vfc0V0dFVHSlEiLCJpc3MiOiJ3ZWIiLCJzayI6IjAiLCJzdHkiOjk5LCJ3Y2QiOiJ1czAyIiwiY2x0IjowLCJleHAiOjE2Nzk1NTk0NDQsImlhdCI6MTY3OTU1MjI0NCwiYWlkIjoiN1MwMXlMSUpRZ1NoNGd4cmplY0JoQSIsImNpZCI6IiJ9.vq5RQcCqe4lAmWtEw3OXJrpAbY3N-FloKGVTS0zGHDw";
	////startMeetingWithoutLoginParam.join_token = NULL;
	//startMeetingWithoutLoginParam.vanityID = NULL;
	//startMeetingWithoutLoginParam.customer_key = NULL;
	////startMeetingWithoutLoginParam.webinarToken = NULL;
	////startMeetingWithoutLoginParam.app_privilege_token = NULL;
	//startMeetingWithoutLoginParam.hDirectShareAppWnd = NULL;
	//startMeetingWithoutLoginParam.isAudioOff = true;
	//startMeetingWithoutLoginParam.isVideoOff = true;
	//startMeetingWithoutLoginParam.isDirectShareDesktop = false;

	//startMeetingParam.param.withoutloginStart = startMeetingWithoutLoginParam;


	////start meeting
	//if ((err = meetingService->Start(startMeetingParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	//else std::cout << "Joining Meeting..." << std::endl;
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
		authContext.jwt_token = GetSignatureFromWebService(WStringToString(webservice_endpoint));


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
	//initParam.strWebDomain = L"https://dev-integration.zoomdev.us/";
	initParam.strWebDomain = L"https://zoom.us/";
	initParam.enableLogByDefault = true;
	if ((err = InitSDK(initParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "SDK Initialized." << std::endl;
	if ((err = CreateNetworkConnectionHelper(&network_connection_helper)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "CreateNetworkConnectionHelper created." << std::endl;
	if ((err = network_connection_helper->RegisterNetworkConnectionHandler(new NetworkConnectionHandler(&SDKAuth))) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	std::cout << "NetworkConnectionHandler registered. Detecting proxy." << std::endl;
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

