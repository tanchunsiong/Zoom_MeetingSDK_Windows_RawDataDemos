#include <windows.h>

#include <fstream>
#include <iostream>
#include <string>

#include "json/json.h"
#include "zoom_sdk.h"
#include "auth_service_interface.h"
#include "direct_share_helper_interface.h"
#include "meeting_service_interface.h"
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
	std::wstring userName = L"Direct Share Demo";
	bool autoTryMeetingNumber = true;
};

bool g_exit = false;
AppConfig g_config;
IAuthService* g_authService = nullptr;
IMeetingService* g_meetingService = nullptr;
INetworkConnectionHelper* g_networkHelper = nullptr;
IDirectShareServiceHelper* g_directShareHelper = nullptr;
AuthServiceEventListener* g_authListener = nullptr;
MeetingServiceEventListener* g_meetingListener = nullptr;
INetworkConnectionHandler* g_networkHandler = nullptr;

class DirectShareEvent final : public IDirectShareServiceHelperEvent
{
public:
	void OnDirectShareStatusUpdate(DirectShareStatus status, IDirectShareViaMeetingIDOrPairingCodeHandler* handler) override
	{
		std::cout << "Direct share status: " << static_cast<int>(status) << std::endl;

		if (status == DirectShare_Need_MeetingID_Or_PairingCode && handler && g_config.autoTryMeetingNumber)
		{
			const SDKError err = handler->TryWithMeetingNumber(g_config.meetingNumber);
			std::cout << "TryWithMeetingNumber result: " << static_cast<int>(err) << std::endl;
		}
	}

	void OnDirectShareSpecifyContent(IDirectShareSpecifyContentHandler* handler) override
	{
		std::cout << "Direct share specify-content callback received; handler=" << handler << std::endl;
	}
};

DirectShareEvent g_directShareEvent;

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
			std::cout << "config.json is invalid; using blank configuration." << std::endl;
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
			g_config.meetingNumber = 0;
		}
	}
	g_config.passcode = root.isMember("passcode") ? StringToWString(root["passcode"].asString()) : L"";
	g_config.userName = root.isMember("user_name") && !root["user_name"].asString().empty()
		? StringToWString(root["user_name"].asString())
		: L"Direct Share Demo";
	g_config.autoTryMeetingNumber = root.isMember("auto_try_meeting_number")
		? root["auto_try_meeting_number"].asBool()
		: true;
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
		[]() { std::cout << "In meeting." << std::endl; });
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
	if (err != SDKERR_SUCCESS)
	{
		Stop("IMeetingService::Join", err);
	}
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

	g_directShareHelper = g_authService->GetDirectShareServiceHelper();
	if (!g_directShareHelper)
	{
		std::cout << "Direct share helper is null. Direct share normally requires a logged-in account." << std::endl;
	}
	else
	{
		err = g_directShareHelper->SetEvent(&g_directShareEvent);
		std::cout << "Direct share helper event result: " << static_cast<int>(err) << std::endl;

		err = g_directShareHelper->CanStartDirectShare();
		std::cout << "CanStartDirectShare result: " << static_cast<int>(err) << std::endl;
		if (err == SDKERR_SUCCESS && !g_directShareHelper->IsDirectShareInProgress())
		{
			err = g_directShareHelper->StartDirectShare();
			std::cout << "StartDirectShare result: " << static_cast<int>(err) << std::endl;
		}
	}

	AuthContext context{};
	context.jwt_token = g_config.sdkJwt.c_str();
	err = g_authService->SDKAuth(context);
	if (err != SDKERR_SUCCESS)
	{
		Stop("IAuthService::SDKAuth", err);
	}
}

void InitSdk()
{
	InitParam init{};
	init.strWebDomain = L"https://zoom.us/";
	init.enableLogByDefault = true;
	init.obConfigOpts.sdkPathPostfix = L"DirectShareDemo";

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
	if (err != SDKERR_SUCCESS)
	{
		Stop("RegisterNetworkConnectionHandler", err);
	}
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
		if (result == -1)
		{
			break;
		}
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
