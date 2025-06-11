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
#include "meeting_service_components/meeting_participants_ctrl_interface.h"
#include <codecvt> //for codecvt
#include "MSDK_BreakoutDemo.h"
#include "IMeetingBoControllerEventListener.h"

using namespace std;
using namespace Json;
using namespace ZOOMSDK;

bool g_exit = false;


IMeetingService* meetingService;
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

bool isJWTWebService = false;
bool isStartMeeting = true;



 

inline bool IsInMeeting(ZOOM_SDK_NAMESPACE::MeetingStatus status)
{
	bool bInMeeting(false);
	if (status == ZOOM_SDK_NAMESPACE::MEETING_STATUS_INMEETING)
	{

		IMeetingBOControllerEventListener* breakoutListener = new IMeetingBOControllerEventListener();
		meetingService->GetMeetingBOController()->SetEvent(breakoutListener);

		//get breakout admin
		IBOAdmin* boAdmin = meetingService->GetMeetingBOController()->GetBOAdminHelper();

		IBOCreator* boCreator = meetingService->GetMeetingBOController()->GetBOCreatorHelper();
		
		IBOData* boData = meetingService->GetMeetingBOController()->GetBODataHelper(); 
		IList<const zchar_t*>* meetingIDList = boData->GetBOMeetingIDList();
		if (meetingIDList != nullptr) {
			for (int i = 0; i < meetingIDList->GetCount(); ++i) {
				const zchar_t* id = meetingIDList->GetItem(i);
			
				IBOMeeting* ibomeeting = boData->GetBOMeetingByID(id);
				ibomeeting->GetBOID();
				ibomeeting->GetBOName();
				ibomeeting->GetBOUserList();
				ibomeeting->GetBOUserStatus(id);
			}
		}


	
		IList<const zchar_t*>* unassignedUserList = boData->GetUnassignedUserList();
		if (unassignedUserList != nullptr) {
			for (int i = 0; i < unassignedUserList->GetCount(); ++i) {
				const zchar_t* id = unassignedUserList->GetItem(i);
			
				boData->GetBOUserName(id);
				boData->IsBOUserMyself(id);
			
			}
		}
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


}

void onMeetingEndsQuitApp() {
	g_exit = true;
}

void onMeetingJoined() {

	printf("Joining Meeting...\n");


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
	meetingService->GetMeetingParticipantsController()->SetEvent(new SkeletonDemo());
	//before joining a meeting, create the setting service
	ISettingService* settingservice;
	CreateSettingService(&settingservice);

	//use the setting service to get the audiosetting


	//use the audiosetting to set EnableAlwaysMuteMicWhenJoinVoip
	IAudioSettingContext* audioSetting;
	audioSetting = settingservice->GetAudioSettings();
	audioSetting->EnableAlwaysMuteMicWhenJoinVoip(true);

	if (!isStartMeeting) {
	
		
	


	ZOOM_SDK_NAMESPACE::JoinParam joinMeetingParam;
	JoinParam4WithoutLogin joinMeetingWithoutLoginParam = joinMeetingParam.param.withoutloginuserJoin;
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
	
	

	//join meeting
	if ((err = meetingService->Join(joinMeetingParam)) != SDKError::SDKERR_SUCCESS) ShowErrorAndExit(err);
	else std::cout << "Joining Meeting..." << std::endl;

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
		startMeetingWithoutLoginParam.userZAK = L"eyJ0eXAiOiJKV1QiLCJzdiI6IjAwMDAwMSIsInptX3NrbSI6InptX28ybSIsImFsZyI6IkhTMjU2In0.eyJhdWQiOiJjbGllbnRzbSIsInVpZCI6InBKOFpHa0FFUXNTTDd4eVJpYVpTVkEiLCJpc3MiOiJ3ZWIiLCJzayI6IjAiLCJzdHkiOjEwMSwid2NkIjoidXMwNSIsImNsdCI6MCwiZXhwIjoxNjk1ODc4NTMwLCJpYXQiOjE2OTU4NzEzMzAsImFpZCI6IjlzU19QQjJ5UjgyN25tdHNzZzl3Z1EiLCJjaWQiOiIifQ.n38PrCFE8CxAQK8_6TaN3HS8Snql0I0z4i6hQJn2HgI";
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
		else std::cout << "Joining Meeting..." << std::endl;
	}
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
	
	//isJWTWebService
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

	string randy = to_string(rand() % 2147483646 + 1);
	const wchar_t* random = wstring(randy.begin(), randy.end()).c_str();



	InitParam initParam;
	//initParam.strWebDomain = L"https://dev-integration.zoomdev.us/";
	initParam.strWebDomain = L"https://zoom.us/";
	initParam.enableLogByDefault = true;
	initParam.obConfigOpts.sdkPathPostfix = random;

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

void SkeletonDemo::onUserJoin(IList<unsigned int>* lstUserID, const zchar_t* strUserList)
{
	
		if (lstUserID && meetingService)
		{
			int count = lstUserID->GetCount();
			for (int i = 0; i < count; i++)
			{
				int userId = lstUserID->GetItem(i);
				IUserInfo* pUserInfo = meetingService->GetMeetingParticipantsController()->GetUserByUserID(userId);
			
				if (pUserInfo)
				{
					printf("UserID %d\n", pUserInfo->GetUserID());
					printf("UserName %ls\n", pUserInfo->GetUserNameA());
					if (pUserInfo->IsHost()) {
						printf("Is Host: true\n");
					}
					else {
						printf("Is Host: false\n");
					}
				}
			}
		}

}

void SkeletonDemo::onUserLeft(IList<unsigned int>* lstUserID, const zchar_t* strUserList)
{
}

void SkeletonDemo::onHostChangeNotification(unsigned int userId)
{
}

void SkeletonDemo::onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid)
{
}

void SkeletonDemo::onUserNamesChanged(IList<unsigned int>* lstUserID)
{
}

void SkeletonDemo::onCoHostChangeNotification(unsigned int userId, bool isCoHost)
{
}

void SkeletonDemo::onInvalidReclaimHostkey()
{
}

void SkeletonDemo::onAllHandsLowered()
{
}

void SkeletonDemo::onLocalRecordingStatusChanged(unsigned int user_id, RecordingStatus status)
{
}

void SkeletonDemo::onAllowParticipantsRenameNotification(bool bAllow)
{
}

void SkeletonDemo::onAllowParticipantsUnmuteSelfNotification(bool bAllow)
{
}

void SkeletonDemo::onAllowParticipantsStartVideoNotification(bool bAllow)
{
}

void SkeletonDemo::onAllowParticipantsShareWhiteBoardNotification(bool bAllow)
{
}

void SkeletonDemo::onRequestLocalRecordingPrivilegeChanged(LocalRecordingRequestPrivilegeStatus status)
{
}

void SkeletonDemo::onAllowParticipantsRequestCloudRecording(bool bAllow)
{
}


void SkeletonDemo::onInMeetingUserAvatarPathUpdated(unsigned int userID)
{
}

void SkeletonDemo::onParticipantProfilePictureStatusChange(bool bHidden)
{
}

void SkeletonDemo::onFocusModeStateChanged(bool bEnabled)
{
}

void SkeletonDemo::onFocusModeShareTypeChanged(FocusModeShareType type)
{
}

void SkeletonDemo::onBotAuthorizerRelationChanged(unsigned int authorizeUserID)
{
}

void SkeletonDemo::onVirtualNameTagStatusChanged(bool bOn, unsigned int userID)
{
}

void SkeletonDemo::onVirtualNameTagRosterInfoUpdated(unsigned int userID)
{
}

void SkeletonDemo::onCreateCompanionRelation(unsigned int parentUserID, unsigned int childUserID)
{
}

void SkeletonDemo::onRemoveCompanionRelation(unsigned int childUserID)
{
}