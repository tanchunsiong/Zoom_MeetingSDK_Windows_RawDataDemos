#include <windows.h>

#include <fstream>
#include <iostream>
#include <string>

#include "json/json.h"
#include "zoom_sdk.h"
#include "meeting_service_interface.h"
#include "meeting_service_components/meeting_audio_interface.h"
#include "meeting_service_components/meeting_participants_ctrl_interface.h"
#include "meeting_service_components/meeting_whiteboard_ctrl_interface.h"
#include "network_connection_handler_interface.h"

#include "..\SDKCompatEventAdapters.h"
#include "AuthServiceEventListener.h"
#include "MeetingServiceEventListener.h"
#include "NetworkConnectionHandler.h"

using namespace Json;
using namespace ZOOMSDK;

namespace
{
constexpr const char* kConfigFile = "config.json";

struct AppConfig
{
	std::wstring sdkJwt;
	UINT64 meetingNumber = 0;
	std::wstring passcode;
	std::wstring userName = L"Whiteboard Demo";
	bool applyParticipantSettings = false;
};

bool g_exit = false;
AppConfig g_config;
IAuthService* g_authService = nullptr;
IMeetingService* g_meetingService = nullptr;
INetworkConnectionHelper* g_networkHelper = nullptr;
IMeetingWhiteboardController* g_whiteboardController = nullptr;
AuthServiceEventListener* g_authListener = nullptr;
MeetingServiceEventListener* g_meetingListener = nullptr;
INetworkConnectionHandler* g_networkHandler = nullptr;

class WhiteboardEvent final : public IMeetingWhiteboardCtrlEvent
{
public:
	void onWhiteboardStatusChanged(SDKWhiteboardStatus status) override
	{
		std::cout << "Whiteboard status changed: "
			<< (status == SDKWhiteboardStatus_Started ? "started" : "stopped") << std::endl;
	}

	void onWhiteboardSettingsChanged(SDKWhiteboardShareOption shareOption, SDKWhiteboardCreateOption createOption, bool enabled) override
	{
		std::cout << "Whiteboard settings changed: share=" << static_cast<int>(shareOption)
			<< ", create=" << static_cast<int>(createOption)
			<< ", participant_create_without_host=" << (enabled ? "true" : "false") << std::endl;
	}
};

WhiteboardEvent g_whiteboardEvent;

std::wstring StringToWString(const std::string& value)
{
	return std::wstring(value.begin(), value.end());
}

void Stop(const char* action, SDKError err)
{
	std::cout << action << " failed: " << static_cast<int>(err) << std::endl;
	g_exit = true;
	PostQuitMessage(0);
}

void ConfigureWhiteboard()
{
	g_whiteboardController = g_meetingService->GetMeetingWhiteboardController();
	if (!g_whiteboardController)
	{
		std::cout << "GetMeetingWhiteboardController returned nullptr." << std::endl;
		return;
	}

	SDKError err = g_whiteboardController->SetEvent(&g_whiteboardEvent);
	std::cout << "Whiteboard SetEvent: " << static_cast<int>(err) << std::endl;
	std::cout << "IsSupportWhiteBoard: " << (g_whiteboardController->IsSupportWhiteBoard() ? "true" : "false") << std::endl;

	CannotShareReasonType cannotShareReason = CannotShareReasonType::CannotShareReasonType_None;
	const bool canShare = g_whiteboardController->CanStartShareWhiteboard(cannotShareReason);
	std::cout << "CanStartShareWhiteboard: " << (canShare ? "true" : "false")
		<< ", reason=" << static_cast<int>(cannotShareReason) << std::endl;

	SDKWhiteboardShareOption shareOption = SDKWhiteboardShareOption_HostShare;
	SDKWhiteboardCreateOption createOption = SDKWhiteboardCreateOption_HostOnly;
	err = g_whiteboardController->GetWhiteboardShareOption(shareOption);
	std::cout << "Current share option: " << static_cast<int>(shareOption)
		<< ", result=" << static_cast<int>(err) << std::endl;
	err = g_whiteboardController->GetWhiteboardCreateOption(createOption);
	std::cout << "Current create option: " << static_cast<int>(createOption)
		<< ", result=" << static_cast<int>(err) << std::endl;
	std::cout << "Other user sharing: " << (g_whiteboardController->IsOtherSharingWhiteboard() ? "true" : "false") << std::endl;

	if (g_config.applyParticipantSettings)
	{
		err = g_whiteboardController->SetWhiteboardShareOption(SDKWhiteboardShareOption_AllGrabShare);
		std::cout << "SetWhiteboardShareOption: " << static_cast<int>(err) << std::endl;
		err = g_whiteboardController->SetWhiteboardCreateOption(SDKWhiteboardCreateOption_All);
		std::cout << "SetWhiteboardCreateOption: " << static_cast<int>(err) << std::endl;
		err = g_whiteboardController->EnableParticipantsCreateWithoutHost(true);
		std::cout << "EnableParticipantsCreateWithoutHost: " << static_cast<int>(err) << std::endl;
	}
}

void LoadConfig()
{
	std::ifstream input(kConfigFile);
	Value root;
	if (input.is_open())
	{
		try
		{
			input >> root;
		}
		catch (const std::exception&)
		{
			std::cout << "config.json is invalid." << std::endl;
		}
	}

	g_config.sdkJwt = root.isMember("sdk_jwt") ? StringToWString(root["sdk_jwt"].asString()) : L"";
	if (root.isMember("meeting_number"))
	{
		try
		{
			g_config.meetingNumber = root["meeting_number"].isUInt64()
				? root["meeting_number"].asUInt64()
				: std::stoull(root["meeting_number"].asString());
		}
		catch (const std::exception&)
		{
		}
	}
	g_config.passcode = root.isMember("passcode") ? StringToWString(root["passcode"].asString()) : L"";
	g_config.userName = root.isMember("user_name") && !root["user_name"].asString().empty()
		? StringToWString(root["user_name"].asString())
		: L"Whiteboard Demo";
	g_config.applyParticipantSettings = root.isMember("apply_participant_settings")
		? root["apply_participant_settings"].asBool()
		: false;
}

void JoinMeeting()
{
	SDKError err = CreateMeetingService(&g_meetingService);
	if (err != SDKERR_SUCCESS)
	{
		Stop("CreateMeetingService", err);
		return;
	}

	g_meetingListener = new MeetingServiceEventListener(
		[]() {},
		[]() { g_exit = true; PostQuitMessage(0); },
		[]()
		{
			std::cout << "In meeting; initializing whiteboard observer." << std::endl;
			ConfigureWhiteboard();
		});
	g_meetingService->SetEvent(g_meetingListener);

	JoinParam param{};
	param.userType = SDK_UT_WITHOUT_LOGIN;
	JoinParam4WithoutLogin& join = param.param.withoutloginuserJoin;
	join.meetingNumber = g_config.meetingNumber;
	join.psw = g_config.passcode.c_str();
	join.userName = g_config.userName.c_str();
	join.userZAK = L"";
	join.customer_key = L"";
	join.isAudioOff = false;
	join.isVideoOff = false;

	err = g_meetingService->Join(param);
	if (err != SDKERR_SUCCESS) Stop("IMeetingService::Join", err);
}

void Authenticate()
{
	SDKError err = CreateAuthService(&g_authService);
	if (err != SDKERR_SUCCESS)
	{
		Stop("CreateAuthService", err);
		return;
	}

	g_authListener = new AuthServiceEventListener(&JoinMeeting);
	err = g_authService->SetEvent(g_authListener);
	if (err != SDKERR_SUCCESS)
	{
		Stop("IAuthService::SetEvent", err);
		return;
	}

	AuthContext context{};
	context.jwt_token = g_config.sdkJwt.c_str();
	err = g_authService->SDKAuth(context);
	if (err != SDKERR_SUCCESS) Stop("IAuthService::SDKAuth", err);
}

void InitSdk()
{
	InitParam init{};
	init.strWebDomain = L"https://zoom.us/";
	init.enableLogByDefault = true;
	init.obConfigOpts.sdkPathPostfix = L"WhiteboardDemo";

	SDKError err = InitSDK(init);
	if (err != SDKERR_SUCCESS)
	{
		Stop("InitSDK", err);
		return;
	}

	err = CreateNetworkConnectionHelper(&g_networkHelper);
	if (err != SDKERR_SUCCESS)
	{
		Stop("CreateNetworkConnectionHelper", err);
		return;
	}

	g_networkHandler = new NetworkConnectionHandler(&Authenticate);
	err = g_networkHelper->RegisterNetworkConnectionHandler(g_networkHandler);
	if (err != SDKERR_SUCCESS) Stop("RegisterNetworkConnectionHandler", err);
}
}

int main()
{
	LoadConfig();
	InitSdk();

	MSG message{};
	int result = 0;
	while (!g_exit && (result = GetMessage(&message, nullptr, 0, 0)) != 0)
	{
		if (result == -1) break;
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if (g_meetingService) DestroyMeetingService(g_meetingService);
	if (g_authService) DestroyAuthService(g_authService);
	if (g_networkHelper) DestroyNetworkConnectionHelper(g_networkHelper);
	delete g_authListener;
	delete g_meetingListener;
	delete g_networkHandler;
	CleanUPSDK();
	return 0;
}
