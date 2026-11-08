#include <windows.h>

#include <fstream>
#include <iostream>
#include <string>

#include "json/json.h"
#include "zoom_sdk.h"
#include "meeting_service_interface.h"
#include "meeting_service_components/meeting_audio_interface.h"
#include "meeting_service_components/meeting_participants_ctrl_interface.h"
#include "meeting_service_components/meeting_waiting_room_interface.h"
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
	std::wstring userName = L"Waiting Room Demo";
	bool enableOnEntry = false;
	bool admitAllOnJoin = false;
};

bool g_exit = false;
AppConfig g_config;
IAuthService* g_authService = nullptr;
IMeetingService* g_meetingService = nullptr;
INetworkConnectionHelper* g_networkHelper = nullptr;
IMeetingWaitingRoomController* g_waitingRoomController = nullptr;
AuthServiceEventListener* g_authListener = nullptr;
MeetingServiceEventListener* g_meetingListener = nullptr;
INetworkConnectionHandler* g_networkHandler = nullptr;

class WaitingRoomEvent final : public IMeetingWaitingRoomEvent
{
public:
	void onWaitingRoomUserJoin(unsigned int userId) override
	{
		std::wcout << L"Waiting-room user joined: " << userId << std::endl;
		LogUser(userId);
	}

	void onWaitingRoomUserLeft(unsigned int userId) override
	{
		std::wcout << L"Waiting-room user left: " << userId << std::endl;
	}

	void onWaitingRoomPresetAudioStatusChanged(bool audioCanTurnOn) override
	{
		std::cout << "Waiting-room audio preset: " << (audioCanTurnOn ? "can turn on" : "muted") << std::endl;
	}

	void onWaitingRoomPresetVideoStatusChanged(bool videoCanTurnOn) override
	{
		std::cout << "Waiting-room video preset: " << (videoCanTurnOn ? "can turn on" : "off") << std::endl;
	}

	void onCustomWaitingRoomDataUpdated(CustomWaitingRoomData& data, IWaitingRoomDataDownloadHandler* handler) override
	{
		std::wcout << L"Custom waiting-room data status: " << static_cast<int>(data.status)
			<< L", title=" << (data.title ? data.title : L"(null)") << std::endl;
		if (data.status == CustomWaitingRoomDataStatus_Download_Failed && handler)
		{
			std::cout << "Custom data download failed; call Retry() or Ignore() as appropriate." << std::endl;
		}
	}

	void onWaitingRoomUserNameChanged(unsigned int userId, const zchar_t* userName) override
	{
		std::wcout << L"Waiting-room name changed: user=" << userId
			<< L", name=" << (userName ? userName : L"(null)") << std::endl;
	}

	void onWaitingRoomEntranceEnabled(bool enabled) override
	{
		std::cout << "Waiting-room entrance " << (enabled ? "enabled" : "disabled") << std::endl;
	}

private:
	static void LogUser(unsigned int userId)
	{
		if (!g_waitingRoomController) return;
		IUserInfo* user = g_waitingRoomController->GetWaitingRoomUserInfoByID(userId);
		std::wcout << L"  id=" << userId << L", name="
			<< (user && user->GetUserName() ? user->GetUserName() : L"(unknown)") << std::endl;
	}
};

WaitingRoomEvent g_waitingRoomEvent;

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

void ConfigureWaitingRoom()
{
	g_waitingRoomController = g_meetingService->GetMeetingWaitingRoomController();
	if (!g_waitingRoomController)
	{
		std::cout << "GetMeetingWaitingRoomController returned nullptr." << std::endl;
		return;
	}

	SDKError err = g_waitingRoomController->SetEvent(&g_waitingRoomEvent);
	std::cout << "Waiting room SetEvent: " << static_cast<int>(err) << std::endl;
	std::cout << "IsSupportWaitingRoom: "
		<< (g_waitingRoomController->IsSupportWaitingRoom() ? "true" : "false") << std::endl;
	std::cout << "Initial on-entry state: "
		<< (g_waitingRoomController->IsWaitingRoomOnEntryFlagOn() ? "enabled" : "disabled") << std::endl;
	std::cout << "On-entry locked: "
		<< (g_waitingRoomController->IsWaitingRoomOnEntryLocked() ? "true" : "false") << std::endl;

	if (g_config.enableOnEntry)
	{
		err = g_waitingRoomController->EnableWaitingRoomOnEntry(true);
		std::cout << "EnableWaitingRoomOnEntry(true): " << static_cast<int>(err) << std::endl;
	}

	IList<unsigned int>* waitingUsers = g_waitingRoomController->GetWaitingRoomLst();
	if (waitingUsers)
	{
		std::cout << "Waiting-room users: " << waitingUsers->GetCount() << std::endl;
		for (int i = 0; i < waitingUsers->GetCount(); ++i)
		{
			const unsigned int userId = waitingUsers->GetItem(i);
			IUserInfo* user = g_waitingRoomController->GetWaitingRoomUserInfoByID(userId);
			std::wcout << L"  id=" << userId << L", name="
				<< (user && user->GetUserName() ? user->GetUserName() : L"(unknown)") << std::endl;
		}
	}

	if (g_config.admitAllOnJoin)
	{
		err = g_waitingRoomController->AdmitAllToMeeting();
		std::cout << "AdmitAllToMeeting: " << static_cast<int>(err) << std::endl;
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
		: L"Waiting Room Demo";
	g_config.enableOnEntry = root.isMember("enable_waiting_room_on_entry")
		? root["enable_waiting_room_on_entry"].asBool()
		: false;
	g_config.admitAllOnJoin = root.isMember("admit_all_on_join")
		? root["admit_all_on_join"].asBool()
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
			std::cout << "Meeting status changed; initializing waiting-room observer." << std::endl;
			ConfigureWaitingRoom();
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
	init.obConfigOpts.sdkPathPostfix = L"WaitingRoomDemo";

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
