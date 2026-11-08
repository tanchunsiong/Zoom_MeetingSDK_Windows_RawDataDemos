// PKCEAuthDemo - Meeting SDK auth via OAuth PKCE (public client, no JWT).
//
// Run with no arguments: generates the PKCE pair, opens the browser, and
// captures the authorization code (manual paste, or the built-in loopback
// listener when callback_mode is "loopback").
// The second half (token exchange -> ZAK -> SDK auth -> start meeting) runs
// in the same process once the code is captured.

#include <iostream>
#include <fstream>
#include <crtdbg.h>
#include <windows.h>

#include <zoom_sdk.h>
#include <meeting_service_interface.h>
#include <network_connection_handler_interface.h>

#include "json/json.h"
#include <curl/curl.h>
#include "OAuthFlow.h"
#include "CodeChallengeHelper.h"
#include "AuthServiceEventListener.h"
#include "MeetingServiceEventListener.h"
#include "NetworkConnectionHandler.h"

using namespace std;
using namespace ZOOMSDK;

static IMeetingService* meetingService = nullptr;
static IAuthService* authService = nullptr;
static INetworkConnectionHelper* networkHelper = nullptr;

static void __cdecl QuietInvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
{
}
static PkceConfig g_cfg;
static wstring g_publicAppKeyW;
static wstring g_zakW;
static wstring g_userNameW;
static ZOOMSDK::StartParam g_startParam;
static bool g_startParamReady = false;
static bool g_exit = false;

static void QuitSoon()
{
	g_exit = true;
	PostQuitMessage(0);
}

static void OnAuthCompleted()
{
	if (!g_startParamReady || !meetingService)
	{
		cout << "Auth completed but start parameters are missing." << endl;
		QuitSoon();
		return;
	}
	const SDKError err = meetingService->Start(g_startParam);
	if (err != SDKError::SDKERR_SUCCESS)
	{
		cout << "Start() failed: " << (int)err << endl;
		QuitSoon();
	}
	else
	{
		cout << "Start() accepted; meeting window should appear." << endl;
	}
}

static void SDKAuth()
{
	SDKError err = SDKError::SDKERR_SUCCESS;
	if ((err = CreateAuthService(&authService)) != SDKError::SDKERR_SUCCESS)
	{
		cout << "CreateAuthService failed: " << (int)err << endl;
		QuitSoon();
		return;
	}
	if ((err = authService->SetEvent(new AuthServiceEventListener(OnAuthCompleted))) != SDKError::SDKERR_SUCCESS)
	{
		cout << "AuthService SetEvent failed: " << (int)err << endl;
		QuitSoon();
		return;
	}

	// Public-client SDK auth: publicAppKey only, no jwt_token.
	AuthContext authContext;
	authContext.publicAppKey = g_publicAppKeyW.c_str();
	cout << "Authenticating SDK with the public app key..." << endl;
	if ((err = authService->SDKAuth(authContext)) != SDKError::SDKERR_SUCCESS)
	{
		cout << "SDKAuth call failed: " << (int)err << endl;
		QuitSoon();
	}
}

static void OnMeetingEnded()
{
	cout << "Meeting ended." << endl;
	QuitSoon();
}

static void OnInMeeting()
{
	cout << "In meeting." << endl;
}

static void OnMeetingStatusChanged()
{
}

static bool CreateMeetingServiceAndListeners()
{
	SDKError err = SDKError::SDKERR_SUCCESS;
	if ((err = CreateMeetingService(&meetingService)) != SDKError::SDKERR_SUCCESS)
	{
		cout << "CreateMeetingService failed: " << (int)err << endl;
		return false;
	}
	if ((err = meetingService->SetEvent(new MeetingServiceEventListener(OnMeetingStatusChanged, OnMeetingEnded, OnInMeeting))) != SDKError::SDKERR_SUCCESS)
	{
		cout << "MeetingService SetEvent failed: " << (int)err << endl;
		return false;
	}
	return true;
}

static void PrepareStartParam()
{
	// StartParam4WithoutLogin (meeting_service_interface.h, SDK 7.0.2.34512) has no
	// passcode member: userZAK, userName, zoomuserType, meetingNumber, vanityID,
	// customer_key, then AV flags only. Starting with meetingNumber + userZAK is
	// sufficient here - the ZAK holder is the host, so Zoom does not prompt for a
	// passcode on host start. PkceConfig::passcode is parsed but unused for now.
	memset(&g_startParam, 0, sizeof(g_startParam));
	g_startParam.userType = SDK_UT_WITHOUT_LOGIN;
	g_startParam.param.withoutloginStart.userZAK = g_zakW.c_str();
	if (!g_userNameW.empty())
	{
		g_startParam.param.withoutloginStart.userName = g_userNameW.c_str();
	}
	g_startParam.param.withoutloginStart.zoomuserType = ZoomUserType_EMAIL_LOGIN;
	g_startParam.param.withoutloginStart.meetingNumber = g_cfg.meetingNumber;
	g_startParam.param.withoutloginStart.isVideoOff = g_cfg.videoOff;
	g_startParam.param.withoutloginStart.isAudioOff = g_cfg.audioOff;
	g_startParamReady = true;
}

static bool CompleteOAuth(const string& code, const string& verifier, string& err)
{
	const string redirect = EffectiveRedirectUri(g_cfg);
	cout << "Exchanging the authorization code for tokens..." << endl;
	const string accessToken = RequestAccessToken(g_cfg, redirect, code, verifier, err);
	if (accessToken.empty()) return false;
	cout << "Access token received." << endl;
	cout << "Fetching ZAK..." << endl;
	const string zak = RequestZak(accessToken, err);
	if (zak.empty()) return false;
	cout << "ZAK received (" << zak.size() << " chars)." << endl;
	g_zakW = Utf8ToWide(zak);
	g_publicAppKeyW = Utf8ToWide(g_cfg.publicClientId);
	g_userNameW = Utf8ToWide(g_cfg.userName);
	return true;
}
static bool RegisterAuthPipeline()
{
	SDKError err = SDKError::SDKERR_SUCCESS;
	if ((err = CreateNetworkConnectionHelper(&networkHelper)) != SDKError::SDKERR_SUCCESS)
	{
		cout << "CreateNetworkConnectionHelper failed: " << (int)err << endl;
		return false;
	}
	if ((err = networkHelper->RegisterNetworkConnectionHandler(new NetworkConnectionHandler(SDKAuth))) != SDKError::SDKERR_SUCCESS)
	{
		cout << "RegisterNetworkConnectionHandler failed: " << (int)err << endl;
		return false;
	}
	cout << "Waiting for network check, then SDK auth..." << endl;
	return true;
}

int main(int argc, char** argv)
{
	string err;

	// Initialize the SDK FIRST: the SDK DLLs tear down cleanly only when
	// InitSDK has run, even if we exit early on a failure path.
	InitParam initParam;
	initParam.strWebDomain = L"https://zoom.us/";
	initParam.enableLogByDefault = true;
	SDKError sdkErr = InitSDK(initParam);
	if (sdkErr != SDKError::SDKERR_SUCCESS)
	{
		cout << "InitSDK failed: " << (int)sdkErr << endl;
		return 1;
	}
	cout << "SDK initialized." << endl;

	if (!LoadPkceConfig(L"config.json", g_cfg, err))
	{
		cout << "Config error: " << err << endl;
		CleanUPSDK();
		return 1;
	}
	cout << "Client id: " << g_cfg.publicClientId << endl;
	cout << "Callback mode: " << g_cfg.callbackMode << endl;

	string code;
	string verifier;

	if (argc > 1)
	{
		if (!ExtractCodeFromUrl(argv[1], code, err))
		{
			cout << "Callback error: " << err << endl;
			CleanUPSDK(); return 1;
		}
		if (!LoadVerifierFromFile(verifier))
		{
			cout << "No saved PKCE verifier; run without arguments first." << endl;
			CleanUPSDK(); return 1;
		}
	}
	else
	{
		CodeChallengeHelper::createCodeVerifier();
		if (CodeChallengeHelper::verifier.empty() || !SaveVerifierToFile(CodeChallengeHelper::verifier))
		{
			cout << "Failed to create or persist the PKCE verifier." << endl;
			CleanUPSDK(); return 1;
		}
		verifier = CodeChallengeHelper::verifier;

		const string redirect = EffectiveRedirectUri(g_cfg);
		const string authorizeUrl = BuildAuthorizeUrl(g_cfg, redirect, CodeChallengeHelper::getCodeChallenge());
		cout << endl << "Authorize URL:" << endl << authorizeUrl << endl << endl;
		if (!LaunchBrowserForAuth(authorizeUrl))
		{
			cout << "Could not open the default browser; open the URL above manually." << endl;
		}

		if (g_cfg.callbackMode == "loopback")
		{
			if (!RunLoopbackCallbackListener(g_cfg.loopbackPort, 300, code, err))
			{
				cout << "Callback error: " << err << endl;
				CleanUPSDK(); return 1;
			}
		}
		else
		{
			cout << "Sign in, click Allow, then paste the code= value" << endl;
			cout << "or the entire redirected URL here:" << endl;
			string line;
			getline(cin, line);
			if (!ExtractCodeFromUrl(line, code, err))
			{
				cout << "Input error: " << err << endl;
				CleanUPSDK(); return 1;
			}
		}
	}

	if (!CompleteOAuth(code, verifier, err))
	{
		cout << "OAuth error: " << err << endl;
		CleanUPSDK(); return 1;
	}

	if (!CreateMeetingServiceAndListeners() || !RegisterAuthPipeline())
	{
		CleanUPSDK(); return 1;
	}
	PrepareStartParam();

	MSG msg;
	while (!g_exit && GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (authService) DestroyAuthService(authService);
	if (meetingService) DestroyMeetingService(meetingService);
	if (networkHelper) DestroyNetworkConnectionHelper(networkHelper);
	CleanUPSDK();
	cout << "Done." << endl;
	return 0;
}
