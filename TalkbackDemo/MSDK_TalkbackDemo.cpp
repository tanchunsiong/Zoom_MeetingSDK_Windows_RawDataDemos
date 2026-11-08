#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "json/json.h"
#include "zoom_sdk.h"
#include "meeting_service_interface.h"
#include "meeting_service_components/meeting_audio_interface.h"
#include "meeting_service_components/meeting_participants_ctrl_interface.h"
#include "meeting_service_components/meeting_talkback_ctrl_interface.h"
#include "network_connection_handler_interface.h"
#include "setting_service_interface.h"

#include "..\SDKCompatEventAdapters.h"
#include "AuthServiceEventListener.h"
#include "MeetingServiceEventListener.h"
#include "NetworkConnectionHandler.h"
#include "WebService.h"

using namespace Json;
using namespace ZOOMSDK;

namespace
{
constexpr const char* kConfigFile = "config.json";
constexpr unsigned int kDefaultChannelCount = 1;
constexpr float kDefaultBackgroundVolume = 0.3f;
constexpr unsigned int kDefaultSampleRate = 48000;
constexpr unsigned int kDefaultToneFrequencyHz = 660;
constexpr unsigned int kDefaultToneDurationSeconds = 5;
constexpr double kPi = 3.14159265358979323846;

struct AppConfig
{
	std::wstring sdkJwt;
	UINT64 meetingNumber = 0;
	std::wstring passcode;
	std::wstring webserviceEndpoint;
	std::wstring userName = L"Talkback Demo";
	bool useWebserviceForAuth = false;
	bool startMeeting = false;
	unsigned int channelCount = kDefaultChannelCount;
	float backgroundVolume = kDefaultBackgroundVolume;
	std::vector<unsigned int> inviteUserIds;
	bool autoSendDemoAudio = true;
	unsigned int sampleRate = kDefaultSampleRate;
	ZoomSDKAudioChannel audioChannel = ZoomSDKAudioChannel_Mono;
	unsigned int toneFrequencyHz = kDefaultToneFrequencyHz;
	unsigned int toneDurationSeconds = kDefaultToneDurationSeconds;
};

bool g_exit = false;
AppConfig g_config;

IMeetingService* g_meetingService = nullptr;
IAuthService* g_authService = nullptr;
INetworkConnectionHelper* g_networkConnectionHelper = nullptr;
IMeetingTalkbackController* g_talkbackController = nullptr;

wchar_t* g_webSignature = nullptr;
AuthServiceEventListener* g_authListener = nullptr;
MeetingServiceEventListener* g_meetingServiceListener = nullptr;
INetworkConnectionHandler* g_networkHandler = nullptr;

std::mutex g_channelMutex;
std::vector<std::wstring> g_createdChannels;
std::wstring g_primaryChannelId;
std::atomic<bool> g_audioThreadShouldRun = false;
std::thread g_audioThread;

std::wstring PromptForWideString(const char* question)
{
	std::wstring input;
	std::cout << question;
	std::getline(std::wcin, input);
	return input;
}

std::wstring StringToWString(const std::string& input)
{
	return std::wstring(input.begin(), input.end());
}

std::string WStringToString(const std::wstring& input)
{
	return std::string(input.begin(), input.end());
}

void ShowErrorAndExit(const char* action, SDKError err)
{
	std::cout << action << " failed with SDKError=" << static_cast<int>(err) << std::endl;
	g_exit = true;
	PostQuitMessage(0);
}

const char* TalkbackErrorToString(IMeetingTalkbackCtrlEvent::TalkbackError error)
{
	switch (error)
	{
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_OK:
		return "TALKBACK_ERROR_OK";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_NOPERMISSION:
		return "TALKBACK_ERROR_NOPERMISSION";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_ALREADY_EXIST:
		return "TALKBACK_ERROR_ALREADY_EXIST";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_COUNT_OVERFLOW:
		return "TALKBACK_ERROR_COUNT_OVERFLOW";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_NOT_EXIST:
		return "TALKBACK_ERROR_NOT_EXIST";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_REJECTED:
		return "TALKBACK_ERROR_REJECTED";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_TIMEOUT:
		return "TALKBACK_ERROR_TIMEOUT";
	case IMeetingTalkbackCtrlEvent::TALKBACK_ERROR_UNKNOWN:
	default:
		return "TALKBACK_ERROR_UNKNOWN";
	}
}

void PrintUsersFromList(const char* prefix, IList<unsigned int>* userList)
{
	if (!userList || !g_meetingService)
	{
		return;
	}

	IMeetingParticipantsController* participantsController = g_meetingService->GetMeetingParticipantsController();
	if (!participantsController)
	{
		return;
	}

	for (int i = 0; i < userList->GetCount(); ++i)
	{
		const unsigned int userId = userList->GetItem(i);
		IUserInfo* user = participantsController->GetUserByUserID(userId);
		const zchar_t* userName = user ? user->GetUserName() : L"(unknown)";
		std::wcout << L"[" << StringToWString(prefix) << L"] user_id=" << userId
			<< L", name=" << (userName ? userName : L"(null)") << std::endl;
	}
}

void PrintCurrentParticipants()
{
	if (!g_meetingService)
	{
		return;
	}

	IMeetingParticipantsController* participantsController = g_meetingService->GetMeetingParticipantsController();
	if (!participantsController)
	{
		return;
	}

	IList<unsigned int>* userList = participantsController->GetParticipantsList();
	if (!userList)
	{
		std::cout << "Participants list is not available yet." << std::endl;
		return;
	}

	std::cout << "Current meeting participants:" << std::endl;
	PrintUsersFromList("participant", userList);
}

void DumpChannelInfo(const std::wstring& channelId)
{
	if (!g_talkbackController)
	{
		return;
	}

	IMeetingTalkbackChannel* channel = g_talkbackController->GetChannelByID(channelId.c_str());
	if (!channel)
	{
		std::wcout << L"Talkback channel not found: " << channelId << std::endl;
		return;
	}

	std::wcout << L"Channel " << channelId << L" current users:" << std::endl;
	PrintUsersFromList("channel", channel->GetUserIDList());
}

void DumpChannelList()
{
	if (!g_talkbackController)
	{
		return;
	}

	IList<IMeetingTalkbackChannel*>* channelList = g_talkbackController->GetChannelList();
	if (!channelList || channelList->GetCount() == 0)
	{
		std::cout << "No talkback channels are available yet." << std::endl;
		return;
	}

	for (int i = 0; i < channelList->GetCount(); ++i)
	{
		IMeetingTalkbackChannel* channel = channelList->GetItem(i);
		if (channel && channel->GetChannelID())
		{
			DumpChannelInfo(channel->GetChannelID());
		}
	}
}

void StopDemoAudioThread()
{
	g_audioThreadShouldRun = false;
	if (g_audioThread.joinable())
	{
		g_audioThread.join();
	}
}

void StartDemoAudioThread(const std::wstring& channelId)
{
	if (!g_talkbackController || !g_config.autoSendDemoAudio)
	{
		return;
	}

	StopDemoAudioThread();
	g_audioThreadShouldRun = true;

	const unsigned int sampleRate = g_config.sampleRate;
	const unsigned int frameSamples = std::max(1u, sampleRate / 50);
	const unsigned int totalFrames = std::max(1u, g_config.toneDurationSeconds * 50);
	const unsigned int toneFrequencyHz = g_config.toneFrequencyHz;
	const ZoomSDKAudioChannel audioChannel = g_config.audioChannel;

	g_audioThread = std::thread([channelId, sampleRate, frameSamples, totalFrames, toneFrequencyHz, audioChannel]()
	{
		const int channelMultiplier = (audioChannel == ZoomSDKAudioChannel_Stereo) ? 2 : 1;
		std::vector<int16_t> frame(frameSamples * channelMultiplier, 0);
		double phase = 0.0;
		const double phaseStep = (2.0 * kPi * static_cast<double>(toneFrequencyHz)) / static_cast<double>(sampleRate);

		std::wcout << L"Streaming demo PCM tone to channel " << channelId << L" for "
			<< g_config.toneDurationSeconds << L" second(s)." << std::endl;

		for (unsigned int frameIndex = 0; frameIndex < totalFrames && g_audioThreadShouldRun; ++frameIndex)
		{
			for (unsigned int i = 0; i < frameSamples; ++i)
			{
				const int16_t sample = static_cast<int16_t>(std::sin(phase) * 6000.0);
				phase += phaseStep;
				if (phase > 2.0 * kPi)
				{
					phase -= 2.0 * kPi;
				}

				frame[i * channelMultiplier] = sample;
				if (channelMultiplier == 2)
				{
					frame[i * channelMultiplier + 1] = sample;
				}
			}

			const SDKError err = g_talkbackController->SendAudioDataToChannel(
				channelId.c_str(),
				reinterpret_cast<const char*>(frame.data()),
				static_cast<unsigned int>(frame.size() * sizeof(int16_t)),
				sampleRate,
				audioChannel);

			if (err != SDKERR_SUCCESS)
			{
				std::cout << "SendAudioDataToChannel failed with SDKError=" << static_cast<int>(err) << std::endl;
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}

		g_audioThreadShouldRun = false;
		std::wcout << L"Demo tone stream finished for channel " << channelId << std::endl;
	});
}

void InviteConfiguredUsers(const std::wstring& channelId)
{
	if (!g_talkbackController || g_config.inviteUserIds.empty())
	{
		std::cout << "No invite_user_ids configured; skipping batch invite." << std::endl;
		return;
	}

	SDKError err = g_talkbackController->BeginBatchInviteUsers(channelId.c_str());
	if (err != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("BeginBatchInviteUsers", err);
		return;
	}

	for (unsigned int userId : g_config.inviteUserIds)
	{
		err = g_talkbackController->AddUserToInvite(userId);
		if (err != SDKERR_SUCCESS)
		{
			std::cout << "AddUserToInvite failed for user " << userId
				<< " with SDKError=" << static_cast<int>(err) << std::endl;
		}
	}

	err = g_talkbackController->ExecuteBatchInviteUsers();
	if (err != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("ExecuteBatchInviteUsers", err);
		return;
	}

	std::wcout << L"Submitted batch invite for channel " << channelId << std::endl;
}

void ConfigureChannelAfterCreate(const std::wstring& channelId)
{
	if (!g_talkbackController)
	{
		return;
	}

	const SDKError volumeErr = g_talkbackController->SetChannelBackgroundVolume(channelId.c_str(), g_config.backgroundVolume);
	if (volumeErr != SDKERR_SUCCESS)
	{
		std::cout << "SetChannelBackgroundVolume failed with SDKError=" << static_cast<int>(volumeErr) << std::endl;
	}
	else
	{
		std::wcout << L"Background volume for channel " << channelId
			<< L" set to " << g_config.backgroundVolume << std::endl;
	}

	InviteConfiguredUsers(channelId);
	DumpChannelInfo(channelId);

	if (g_config.autoSendDemoAudio)
	{
		StartDemoAudioThread(channelId);
	}
}

class ParticipantsLogger final : public MeetingParticipantsCtrlEventAdapter
{
public:
	void onUserJoin(IList<unsigned int>* lstUserID, const zchar_t* strUserList = nullptr) override
	{
		(void)strUserList;
		PrintUsersFromList("joined", lstUserID);
	}

	void onUserLeft(IList<unsigned int>* lstUserID, const zchar_t* strUserList = nullptr) override
	{
		(void)strUserList;
		PrintUsersFromList("left", lstUserID);
	}

	void onHostChangeNotification(unsigned int userId) override { (void)userId; }
	void onLowOrRaiseHandStatusChanged(bool bLow, unsigned int userid) override { (void)bLow; (void)userid; }
	void onUserNamesChanged(IList<unsigned int>* lstUserID) override { (void)lstUserID; }
	void onCoHostChangeNotification(unsigned int userId, bool isCoHost) override { (void)userId; (void)isCoHost; }
	void onInvalidReclaimHostkey() override {}
	void onAllHandsLowered() override {}
	void onLocalRecordingStatusChanged(unsigned int user_id, RecordingStatus status) override { (void)user_id; (void)status; }
	void onAllowParticipantsRenameNotification(bool bAllow) override { (void)bAllow; }
	void onAllowParticipantsUnmuteSelfNotification(bool bAllow) override { (void)bAllow; }
	void onAllowParticipantsStartVideoNotification(bool bAllow) override { (void)bAllow; }
	void onAllowParticipantsShareWhiteBoardNotification(bool bAllow) override { (void)bAllow; }
	void onRequestLocalRecordingPrivilegeChanged(LocalRecordingRequestPrivilegeStatus status) override { (void)status; }
	void onAllowParticipantsRequestCloudRecording(bool bAllow) override { (void)bAllow; }
	void onInMeetingUserAvatarPathUpdated(unsigned int userID) override { (void)userID; }
	void onParticipantProfilePictureStatusChange(bool bHidden) override { (void)bHidden; }
	void onFocusModeStateChanged(bool bEnabled) override { (void)bEnabled; }
	void onFocusModeShareTypeChanged(FocusModeShareType type) override { (void)type; }
	void onBotAuthorizerRelationChanged(unsigned int authorizeUserID) override { (void)authorizeUserID; }
	void onVirtualNameTagStatusChanged(bool bOn, unsigned int userID) override { (void)bOn; (void)userID; }
	void onVirtualNameTagRosterInfoUpdated(unsigned int userID) override { (void)userID; }
	void onCreateCompanionRelation(unsigned int parentUserID, unsigned int childUserID) override { (void)parentUserID; (void)childUserID; }
	void onRemoveCompanionRelation(unsigned int childUserID) override { (void)childUserID; }
};

class TalkbackEventHandler final : public IMeetingTalkbackCtrlEvent
{
public:
	void onCreateChannelResponse(const zchar_t* channelID, TalkbackError error) override
	{
		std::wcout << L"onCreateChannelResponse channel=" << (channelID ? channelID : L"(null)")
			<< L", error=" << StringToWString(TalkbackErrorToString(error)) << std::endl;

		if (!channelID || error != TALKBACK_ERROR_OK)
		{
			return;
		}

		const std::wstring createdChannelId(channelID);
		bool isPrimaryChannel = false;
		{
			std::lock_guard<std::mutex> lock(g_channelMutex);
			if (std::find(g_createdChannels.begin(), g_createdChannels.end(), createdChannelId) == g_createdChannels.end())
			{
				g_createdChannels.push_back(createdChannelId);
			}
			if (g_primaryChannelId.empty())
			{
				g_primaryChannelId = createdChannelId;
				isPrimaryChannel = true;
			}
		}

		if (isPrimaryChannel)
		{
			ConfigureChannelAfterCreate(createdChannelId);
		}
		else
		{
			std::wcout << L"Secondary channel created: " << createdChannelId
				<< L". This sample only auto-configures the first channel." << std::endl;
		}
	}

	void onDestroyChannelResponse(const zchar_t* channelID, TalkbackError error) override
	{
		std::wcout << L"onDestroyChannelResponse channel=" << (channelID ? channelID : L"(null)")
			<< L", error=" << StringToWString(TalkbackErrorToString(error)) << std::endl;
	}

	void onChannelUserJoinResponse(const zchar_t* channelID, unsigned int userID, TalkbackError error) override
	{
		std::wcout << L"onChannelUserJoinResponse channel=" << (channelID ? channelID : L"(null)")
			<< L", user_id=" << userID
			<< L", error=" << StringToWString(TalkbackErrorToString(error)) << std::endl;

		if (channelID && error == TALKBACK_ERROR_OK)
		{
			DumpChannelInfo(channelID);
		}
	}

	void onChannelUserLeaveResponse(const zchar_t* channelID, unsigned int userID, TalkbackError error) override
	{
		std::wcout << L"onChannelUserLeaveResponse channel=" << (channelID ? channelID : L"(null)")
			<< L", user_id=" << userID
			<< L", error=" << StringToWString(TalkbackErrorToString(error)) << std::endl;
	}

	void onJoinTalkbackChannel(unsigned int inviterID) override
	{
		std::cout << "onJoinTalkbackChannel inviter_id=" << inviterID << std::endl;
	}

	void onLeaveTalkbackChannel(unsigned int inviterID) override
	{
		std::cout << "onLeaveTalkbackChannel inviter_id=" << inviterID << std::endl;
	}

	void onInviterAudioLevel(unsigned int inviterID, unsigned int audioLevel) override
	{
		std::cout << "onInviterAudioLevel inviter_id=" << inviterID
			<< ", audio_level=" << audioLevel << std::endl;
	}
};

ParticipantsLogger g_participantsLogger;
TalkbackEventHandler g_talkbackEventHandler;

bool ReadOptionalBool(const Value& config, const char* key, bool defaultValue)
{
	return config.isMember(key) ? config[key].asBool() : defaultValue;
}

unsigned int ReadOptionalUInt(const Value& config, const char* key, unsigned int defaultValue)
{
	return config.isMember(key) ? config[key].asUInt() : defaultValue;
}

float ReadOptionalFloat(const Value& config, const char* key, float defaultValue)
{
	return config.isMember(key) ? config[key].asFloat() : defaultValue;
}

std::vector<unsigned int> ReadInviteUserIds(const Value& config)
{
	std::vector<unsigned int> inviteUserIds;
	if (!config.isMember("invite_user_ids") || !config["invite_user_ids"].isArray())
	{
		return inviteUserIds;
	}

	for (const Value& userValue : config["invite_user_ids"])
	{
		if (userValue.isUInt())
		{
			inviteUserIds.push_back(userValue.asUInt());
		}
		else if (userValue.isString())
		{
			try
			{
				inviteUserIds.push_back(static_cast<unsigned int>(std::stoul(userValue.asString())));
			}
			catch (const std::exception&)
			{
				std::cout << "Skipping invalid invite_user_ids entry: "
					<< userValue.asString() << std::endl;
			}
		}
	}

	return inviteUserIds;
}

ZoomSDKAudioChannel ParseAudioChannel(const Value& config)
{
	const std::string channel = config.isMember("audio_channel")
		? config["audio_channel"].asString()
		: "mono";
	return channel == "stereo" ? ZoomSDKAudioChannel_Stereo : ZoomSDKAudioChannel_Mono;
}

void LoadConfig()
{
	std::ifstream configStream(kConfigFile);
	Value config;

	std::cout << "Loading talkback configuration from " << kConfigFile << std::endl;
	if (configStream.is_open())
	{
		try
		{
			configStream >> config;
		}
		catch (const std::exception&)
		{
			std::cout << "config.json is not valid JSON; falling back to interactive prompts for required values." << std::endl;
		}
	}
	else
	{
		std::cout << "config.json was not found; prompting for required fields." << std::endl;
	}

	if (!configStream.is_open() || !config.isMember("sdk_jwt") || config["sdk_jwt"].asString().empty())
	{
		g_config.sdkJwt = PromptForWideString("SDK auth token: ");
	}
	else
	{
		g_config.sdkJwt = StringToWString(config["sdk_jwt"].asString());
	}

	if (!configStream.is_open() || !config.isMember("meeting_number") || config["meeting_number"].asString().empty())
	{
		bool validMeetingNumber = false;
		while (!validMeetingNumber)
		{
			try
			{
				g_config.meetingNumber = std::stoull(WStringToString(PromptForWideString("Meeting Number: ")));
				validMeetingNumber = true;
			}
			catch (const std::exception&)
			{
				std::cout << "Meeting Number must contain only digits." << std::endl;
			}
		}
	}
	else if (config["meeting_number"].isUInt64())
	{
		g_config.meetingNumber = config["meeting_number"].asUInt64();
	}
	else
	{
		g_config.meetingNumber = std::stoull(config["meeting_number"].asString());
	}

	if (!configStream.is_open() || !config.isMember("passcode") || config["passcode"].asString().empty())
	{
		g_config.passcode = PromptForWideString("Passcode: ");
	}
	else
	{
		g_config.passcode = StringToWString(config["passcode"].asString());
	}

	g_config.userName = config.isMember("user_name") && !config["user_name"].asString().empty()
		? StringToWString(config["user_name"].asString())
		: L"Talkback Demo";
	g_config.webserviceEndpoint = config.isMember("webservice_endpoint")
		? StringToWString(config["webservice_endpoint"].asString())
		: L"";
	g_config.useWebserviceForAuth = ReadOptionalBool(config, "use_webservice_for_auth", false);
	g_config.startMeeting = ReadOptionalBool(config, "start_meeting", false);
	g_config.channelCount = std::max(1u, ReadOptionalUInt(config, "channel_count", kDefaultChannelCount));
	g_config.backgroundVolume = ReadOptionalFloat(config, "background_volume", kDefaultBackgroundVolume);
	g_config.inviteUserIds = ReadInviteUserIds(config);
	g_config.autoSendDemoAudio = ReadOptionalBool(config, "auto_send_demo_audio", true);
	g_config.sampleRate = ReadOptionalUInt(config, "sample_rate", kDefaultSampleRate);
	g_config.audioChannel = ParseAudioChannel(config);
	g_config.toneFrequencyHz = ReadOptionalUInt(config, "tone_frequency_hz", kDefaultToneFrequencyHz);
	g_config.toneDurationSeconds = ReadOptionalUInt(config, "tone_duration_seconds", kDefaultToneDurationSeconds);

	std::cout << "Configured channel_count=" << g_config.channelCount
		<< ", background_volume=" << g_config.backgroundVolume
		<< ", invite_user_ids=" << g_config.inviteUserIds.size()
		<< ", auto_send_demo_audio=" << (g_config.autoSendDemoAudio ? "true" : "false")
		<< std::endl;
}

void JoinOrStartMeeting()
{
	SDKError err = SDKERR_SUCCESS;

	if (!g_meetingService)
	{
		err = CreateMeetingService(&g_meetingService);
		if (err != SDKERR_SUCCESS)
		{
			ShowErrorAndExit("CreateMeetingService", err);
			return;
		}
	}

	g_meetingServiceListener = new MeetingServiceEventListener(
		[]() { std::cout << "Meeting parameter notification received." << std::endl; },
		[]()
		{
			std::cout << "Meeting ended; shutting down sample." << std::endl;
			StopDemoAudioThread();
			g_exit = true;
			PostQuitMessage(0);
		},
		[]()
		{
			std::cout << "Entered meeting. Initializing talkback..." << std::endl;
			IMeetingParticipantsController* participantsController =
				g_meetingService ? g_meetingService->GetMeetingParticipantsController() : nullptr;
			if (participantsController)
			{
				participantsController->SetEvent(&g_participantsLogger);
			}
			PrintCurrentParticipants();

			g_talkbackController = g_meetingService ? g_meetingService->GetMeetingTalkbackController() : nullptr;
			if (!g_talkbackController)
			{
				std::cout << "GetMeetingTalkbackController returned nullptr. Confirm the project is using SDK 7.0.x headers and libraries." << std::endl;
				return;
			}

			if (!g_talkbackController->IsMeetingSupportTalkBack())
			{
				std::cout << "This meeting does not report talkback support." << std::endl;
				return;
			}

			const SDKError eventErr = g_talkbackController->SetEvent(&g_talkbackEventHandler);
			if (eventErr != SDKERR_SUCCESS)
			{
				ShowErrorAndExit("IMeetingTalkbackController::SetEvent", eventErr);
				return;
			}

			std::cout << "Talkback controller is ready." << std::endl;
			DumpChannelList();

			const SDKError createErr = g_talkbackController->CreateChannel(g_config.channelCount);
			if (createErr != SDKERR_SUCCESS)
			{
				ShowErrorAndExit("IMeetingTalkbackController::CreateChannel", createErr);
				return;
			}

			std::cout << "Submitted CreateChannel(" << g_config.channelCount
				<< "). Waiting for async callbacks..." << std::endl;
		});

	g_meetingService->SetEvent(g_meetingServiceListener);

	ISettingService* settingService = nullptr;
	err = CreateSettingService(&settingService);
	if (err == SDKERR_SUCCESS && settingService)
	{
		if (IAudioSettingContext* audioSetting = settingService->GetAudioSettings())
		{
			audioSetting->EnableAlwaysMuteMicWhenJoinVoip(true);
		}
		DestroySettingService(settingService);
	}

	if (!g_config.startMeeting)
	{
		JoinParam joinParam{};
		joinParam.userType = SDK_UT_WITHOUT_LOGIN;
		JoinParam4WithoutLogin& withoutLogin = joinParam.param.withoutloginuserJoin;
		withoutLogin.meetingNumber = g_config.meetingNumber;
		withoutLogin.psw = g_config.passcode.c_str();
		withoutLogin.userName = g_config.userName.c_str();
		withoutLogin.userZAK = L"";
		withoutLogin.customer_key = L"";
		withoutLogin.join_token = nullptr;
		withoutLogin.vanityID = nullptr;
		withoutLogin.webinarToken = nullptr;
		withoutLogin.app_privilege_token = nullptr;
		withoutLogin.hDirectShareAppWnd = nullptr;
		withoutLogin.isAudioOff = true;
		withoutLogin.isVideoOff = true;
		withoutLogin.isDirectShareDesktop = false;

		err = g_meetingService->Join(joinParam);
		if (err != SDKERR_SUCCESS)
		{
			ShowErrorAndExit("IMeetingService::Join", err);
			return;
		}

		std::cout << "Joining meeting..." << std::endl;
		return;
	}

	ZOOMSDK::StartParam startParam{};
	startParam.userType = SDK_UT_WITHOUT_LOGIN;
	StartParam4WithoutLogin& withoutLoginStart = startParam.param.withoutloginStart;
	withoutLoginStart.meetingNumber = g_config.meetingNumber;
	withoutLoginStart.zoomuserType = ZoomUserType_APIUSER;
	withoutLoginStart.userName = g_config.userName.c_str();
	withoutLoginStart.userZAK = L"";
	withoutLoginStart.customer_key = nullptr;
	withoutLoginStart.hDirectShareAppWnd = nullptr;
	withoutLoginStart.isAudioOff = true;
	withoutLoginStart.isVideoOff = true;
	withoutLoginStart.isDirectShareDesktop = false;

	err = g_meetingService->Start(startParam);
	if (err != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("IMeetingService::Start", err);
		return;
	}

	std::cout << "Starting meeting..." << std::endl;
}

void SDKAuth()
{
	SDKError err = SDKERR_SUCCESS;

	if (!g_authService)
	{
		err = CreateAuthService(&g_authService);
		if (err != SDKERR_SUCCESS)
		{
			ShowErrorAndExit("CreateAuthService", err);
			return;
		}
	}

	if (!g_authListener)
	{
		g_authListener = new AuthServiceEventListener(&JoinOrStartMeeting);
		err = g_authService->SetEvent(g_authListener);
		if (err != SDKERR_SUCCESS)
		{
			ShowErrorAndExit("IAuthService::SetEvent", err);
			return;
		}
	}

	AuthContext authContext{};
	if (g_config.useWebserviceForAuth)
	{
		delete[] g_webSignature;
		g_webSignature = GetSignatureFromWebService(WStringToString(g_config.webserviceEndpoint));
		authContext.jwt_token = g_webSignature;
	}
	else
	{
		authContext.jwt_token = g_config.sdkJwt.c_str();
	}

	if (!authContext.jwt_token || std::wstring(authContext.jwt_token).empty())
	{
		std::cout << "No SDK auth token is available. Check sdk_jwt or your webservice_endpoint." << std::endl;
		g_exit = true;
		PostQuitMessage(0);
		return;
	}

	err = g_authService->SDKAuth(authContext);
	if (err != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("IAuthService::SDKAuth", err);
		return;
	}

	std::cout << "SDK auth request submitted." << std::endl;
}

void InitSdk()
{
	const std::wstring pathPostfix = L"TalkbackDemo_" + std::to_wstring(GetCurrentProcessId());
	InitParam initParam{};
	initParam.strWebDomain = L"https://zoom.us/";
	initParam.enableLogByDefault = true;
	initParam.obConfigOpts.sdkPathPostfix = pathPostfix.c_str();

	const SDKError initErr = InitSDK(initParam);
	if (initErr != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("InitSDK", initErr);
		return;
	}

	std::cout << "SDK initialized." << std::endl;

	const SDKError helperErr = CreateNetworkConnectionHelper(&g_networkConnectionHelper);
	if (helperErr != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("CreateNetworkConnectionHelper", helperErr);
		return;
	}

	g_networkHandler = new NetworkConnectionHandler(&SDKAuth);
	const SDKError registerErr = g_networkConnectionHelper->RegisterNetworkConnectionHandler(g_networkHandler);
	if (registerErr != SDKERR_SUCCESS)
	{
		ShowErrorAndExit("RegisterNetworkConnectionHandler", registerErr);
		return;
	}

	std::cout << "Proxy detection started." << std::endl;
}
} // namespace

int main()
{
	LoadConfig();
	InitSdk();

	MSG msg{};
	int messageResult = 0;
	while (!g_exit && (messageResult = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (messageResult == -1)
		{
			std::cout << "GetMessage returned -1; exiting." << std::endl;
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	StopDemoAudioThread();

	if (g_meetingService)
	{
		DestroyMeetingService(g_meetingService);
		g_meetingService = nullptr;
	}

	if (g_authService)
	{
		DestroyAuthService(g_authService);
		g_authService = nullptr;
	}

	if (g_networkConnectionHelper)
	{
		DestroyNetworkConnectionHelper(g_networkConnectionHelper);
		g_networkConnectionHelper = nullptr;
	}

	delete g_authListener;
	g_authListener = nullptr;
	delete g_meetingServiceListener;
	g_meetingServiceListener = nullptr;
	delete g_networkHandler;
	g_networkHandler = nullptr;
	delete[] g_webSignature;
	g_webSignature = nullptr;

	CleanUPSDK();
	return 0;
}
