#pragma once

#include <string>

// Public-client PKCE configuration, loaded from config.json. No secrets:
// the token exchange is header-less (client_id in the form body) per the
// pure public-client OAuth flow.
struct PkceConfig
{
	std::string publicClientId;
	std::string redirectUri;      // manual-mode redirect + token-exchange value
	std::string callbackMode;     // "manual" (default) or "loopback"
	int loopbackPort = 0;         // used when callbackMode == "loopback"
	unsigned long long meetingNumber = 0;
	std::string userName;
	bool videoOff = false;
	bool audioOff = false;
	std::string passcode; // parsed for future use; StartParam4WithoutLogin has no passcode member
};

bool LoadPkceConfig(const std::wstring& configPath, PkceConfig& cfg, std::string& err);
std::string EffectiveRedirectUri(const PkceConfig& cfg);
std::string UrlEncode(const std::string& value);
std::string UrlDecode(const std::string& value);
std::string BuildAuthorizeUrl(const PkceConfig& cfg, const std::string& redirectUri, const std::string& codeChallenge);
bool LaunchBrowserForAuth(const std::string& url);
bool ExtractCodeFromUrl(const std::string& urlOrCode, std::string& codeOut, std::string& err);
// Binds 127.0.0.1:port, waits for GET .../callback?code=..., replies 200, returns the code.
bool RunLoopbackCallbackListener(int port, int timeoutSeconds, std::string& codeOut, std::string& err);
std::string RequestAccessToken(const PkceConfig& cfg, const std::string& redirectUri, const std::string& code, const std::string& verifier, std::string& err);
std::string RequestZak(const std::string& accessToken, std::string& err);

std::string WideToUtf8(const std::wstring& w);
std::wstring Utf8ToWide(const std::string& s);

bool SaveVerifierToFile(const std::string& verifier);
bool LoadVerifierFromFile(std::string& verifierOut);
