#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <ctime>

#include <curl/curl.h>
#include <json/json.h>

#include "OAuthFlow.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;
using namespace Json;

static string CaBundlePath()
{
	char path[MAX_PATH] = {};
	GetModuleFileNameA(nullptr, path, MAX_PATH);
	string dir(path);
	const size_t slash = dir.find_last_of('\\');
	if (slash != string::npos) dir.resize(slash);
	return dir + '\\' + "curl-ca-bundle.crt";
}

static size_t WriteCallback(char* contents, size_t size, size_t nmemb, string* response)
{
	response->append(contents, size * nmemb);
	return size * nmemb;
}

string WideToUtf8(const wstring& w)
{
	if (w.empty()) return string();
	const int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
	string out(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &out[0], size, nullptr, nullptr);
	return out;
}

wstring Utf8ToWide(const string& s)
{
	if (s.empty()) return wstring();
	const int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
	wstring out(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &out[0], size);
	return out;
}

bool LoadPkceConfig(const wstring& configPath, PkceConfig& cfg, string& err)
{
	ifstream f(configPath.c_str());
	if (!f.is_open())
	{
		err = "config.json not found next to the executable";
		return false;
	}

	Value root;
	CharReaderBuilder builder;
	string parseErr;
	if (!parseFromStream(builder, f, &root, &parseErr))
	{
		err = "config.json is not valid JSON: " + parseErr;
		return false;
	}

	cfg.publicClientId = root.get("public_client_id", "").asString();
	cfg.redirectUri = root.get("redirect_uri", "").asString();
	cfg.callbackMode = root.get("callback_mode", "manual").asString();
	cfg.loopbackPort = root.get("loopback_port", 53390).asInt();
	cfg.userName = root.get("user_name", "PKCE Demo").asString();
	cfg.videoOff = root.get("video_off", false).asBool();
	cfg.audioOff = root.get("audio_off", false).asBool();
	cfg.passcode = root.get("passcode", "").asString();

	const Value& mn = root["meeting_number"];
	if (!mn.empty())
	{
		if (mn.isNumeric())
		{
			cfg.meetingNumber = mn.asUInt64();
		}
		else
		{
			cfg.meetingNumber = _strtoui64(mn.asString().c_str(), nullptr, 10);
		}
	}

	if (cfg.publicClientId.empty()) { err = "public_client_id is required in config.json"; return false; }
	if (cfg.redirectUri.empty()) { err = "redirect_uri is required in config.json"; return false; }
	if (cfg.meetingNumber == 0) { err = "meeting_number is required in config.json"; return false; }
	if (cfg.callbackMode != "manual" && cfg.callbackMode != "loopback")
	{
		err = "callback_mode must be manual or loopback";
		return false;
	}
	if (cfg.callbackMode == "loopback" && (cfg.loopbackPort <= 0 || cfg.loopbackPort > 65535))
	{
		err = "loopback_port must be 1-65535 in loopback mode";
		return false;
	}
	return true;
}

string EffectiveRedirectUri(const PkceConfig& cfg)
{
	if (cfg.callbackMode == "loopback")
	{
		ostringstream oss;
		oss << "http://127.0.0.1:" << cfg.loopbackPort << "/callback";
		return oss.str();
	}
	return cfg.redirectUri;
}

// Percent-encode per the PKCE guide: keep unreserved chars, encode the rest.
string UrlEncode(const string& value)
{
	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;
	for (const char c : value)
	{
		const unsigned char uc = (unsigned char)c;
		if (isalnum(uc) || uc == '-' || uc == '_' || uc == '.' || uc == '~')
		{
			escaped << c;
		}
		else
		{
			escaped << uppercase << '%' << setw(2) << (int)uc << nouppercase;
		}
	}
	return escaped.str();
}

string UrlDecode(const string& value)
{
	string out;
	for (size_t i = 0; i < value.size(); ++i)
	{
		if (value[i] == '%' && i + 2 < value.size())
		{
			const char hex[3] = { value[i + 1], value[i + 2], 0 };
			out += (char)strtoul(hex, nullptr, 16);
			i += 2;
		}
		else if (value[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += value[i];
		}
	}
	return out;
}

string BuildAuthorizeUrl(const PkceConfig& cfg, const string& redirectUri, const string& codeChallenge)
{
	string url = "https://zoom.us/oauth/authorize";
	url += "?response_type=code";
	url += "&client_id=" + UrlEncode(cfg.publicClientId);
	url += "&code_challenge=" + UrlEncode(codeChallenge);
	url += "&redirect_uri=" + UrlEncode(redirectUri);
	url += "&code_challenge_method=S256";
	return url;
}

bool LaunchBrowserForAuth(const string& url)
{
	const wstring wUrl = Utf8ToWide(url);
	// ShellExecuteW returns a value > 32 on success.
	return (INT_PTR)ShellExecuteW(nullptr, L"open", wUrl.c_str(), nullptr, nullptr, SW_SHOW) > 32;
}

// Accepts a full callback URL or a bare code value pasted by the user.
bool ExtractCodeFromUrl(const string& urlOrCode, string& codeOut, string& err)
{
	const string& s = urlOrCode;
	const size_t queryStart = s.find('?');
	if (queryStart == string::npos)
	{
		if (!s.empty() && s.find(':') == string::npos && s.find(' ') == string::npos)
		{
			codeOut = UrlDecode(s);
			return true;
		}
		err = "no query string found in: " + s;
		return false;
	}

	istringstream stream(s.substr(queryStart + 1));
	string pair;
	while (getline(stream, pair, '&'))
	{
		const size_t eq = pair.find('=');
		if (eq == string::npos) continue;
		const string key = pair.substr(0, eq);
		const string value = UrlDecode(pair.substr(eq + 1));
		if (key == "error")
		{
			err = "authorization failed: " + value;
			return false;
		}
		if (key == "code")
		{
			codeOut = value;
		}
	}
	if (codeOut.empty()) { err = "no code parameter found in: " + s; return false; }
	return true;
}

static bool ParseHttpRequestTarget(const string& request, string& targetOut)
{
	istringstream stream(request);
	string method, target, version;
	if (stream >> method >> target >> version && method == "GET")
	{
		targetOut = target;
		return true;
	}
	return false;
}

bool RunLoopbackCallbackListener(int port, int timeoutSeconds, string& codeOut, string& err)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		err = "WSAStartup failed";
		return false;
	}
	bool ok = false;

	const SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listener == INVALID_SOCKET)
	{
		err = "socket() failed: " + to_string(WSAGetLastError());
	}
	else
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons((u_short)port);
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		if (bind(listener, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR ||
			listen(listener, 1) == SOCKET_ERROR)
		{
			err = "bind/listen failed on 127.0.0.1:" + to_string(port);
		}
		else
		{
			cout << "Listening for the OAuth callback on http://127.0.0.1:" << port << "/callback ..." << endl;
			const time_t deadline = time(nullptr) + timeoutSeconds;
			while (time(nullptr) < deadline)
			{
				fd_set readSet;
				FD_ZERO(&readSet);
				FD_SET(listener, &readSet);
				timeval tv;
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				if (select(0, &readSet, nullptr, nullptr, &tv) <= 0) continue;

				const SOCKET conn = accept(listener, nullptr, nullptr);
				if (conn == INVALID_SOCKET) continue;

				char buf[4096];
				memset(buf, 0, sizeof(buf));
				const int received = recv(conn, buf, (int)sizeof(buf) - 1, 0);
				if (received > 0)
				{
					const char* response =
						"HTTP/1.1 200 OK\r\n"
						"Content-Type: text/html\r\n"
						"Connection: close\r\n"
						"\r\n"
						"<html><body><h2>PKCEAuthDemo</h2><p>Authorization received; close this tab.</p></body></html>";
					send(conn, response, (int)strlen(response), 0);

					string target;
					if (ParseHttpRequestTarget(buf, target))
					{
						string probeErr;
						if (ExtractCodeFromUrl("http://localhost" + target, codeOut, probeErr))
						{
							ok = true;
						}
						else
						{
							err = probeErr;
						}
					}
				}
				closesocket(conn);
				break;
			}
			if (!ok && err.empty()) err = "timed out waiting for the callback";
		}
		closesocket(listener);
	}

	WSACleanup();
	return ok;
}

// Header-less public-client token exchange: client_id travels in the form
// body; no Authorization header, no client secret.
string RequestAccessToken(const PkceConfig& cfg, const string& redirectUri, const string& code, const string& verifier, string& err)
{
	CURL* curl = curl_easy_init();
	if (!curl) { err = "curl init failed"; return string(); }

	string response;
	const string post =
		"grant_type=authorization_code"
		"&code=" + UrlEncode(code) +
		"&client_id=" + UrlEncode(cfg.publicClientId) +
		"&redirect_uri=" + UrlEncode(redirectUri) +
		"&code_verifier=" + UrlEncode(verifier);

	char tokenCurlErrbuf[CURL_ERROR_SIZE] = {};
	curl_easy_setopt(curl, CURLOPT_URL, "https://zoom.us/oauth/token");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, post.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	const string caBundle = CaBundlePath();
	if (GetFileAttributesA(caBundle.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		curl_easy_setopt(curl, CURLOPT_CAINFO, caBundle.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);

	curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	const CURLcode res = curl_easy_perform(curl);
	long httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		err = string("token request failed: ") + curl_easy_strerror(res) + " [" + tokenCurlErrbuf + "]";
		return string();
	}

	Value root;
	CharReaderBuilder builder;
	string parseErr;
	istringstream stream(response);
	if (!parseFromStream(builder, stream, &root, &parseErr))
	{
		err = "token response is not JSON: " + response;
		return string();
	}
	if (httpCode != 200)
	{
		err = "token endpoint HTTP " + to_string(httpCode) + ": " + root.get("error_description", root.get("reason", response).asString()).asString();
		return string();
	}
	const string token = root.get("access_token", "").asString();
	if (token.empty()) err = "token response had no access_token";
	return token;
}

string RequestZak(const string& accessToken, string& err)
{
	if (accessToken.empty()) { err = "no access token"; return string(); }

	CURL* curl = curl_easy_init();
	if (!curl) { err = "curl init failed"; return string(); }

	string response;
	curl_easy_setopt(curl, CURLOPT_URL, "https://api.zoom.us/v2/users/me/token?type=zak");
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	const string caBundle = CaBundlePath();
	if (GetFileAttributesA(caBundle.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		curl_easy_setopt(curl, CURLOPT_CAINFO, caBundle.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);

	curl_slist* headers = nullptr;
	const string authHeader = "Authorization: Bearer " + accessToken;
	headers = curl_slist_append(headers, authHeader.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	const CURLcode res = curl_easy_perform(curl);
	long httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		err = string("ZAK request failed: ") + curl_easy_strerror(res);
		return string();
	}

	Value root;
	CharReaderBuilder builder;
	string parseErr;
	istringstream stream(response);
	if (!parseFromStream(builder, stream, &root, &parseErr))
	{
		err = "ZAK response is not JSON: " + response;
		return string();
	}
	if (httpCode != 200)
	{
		err = "ZAK endpoint HTTP " + to_string(httpCode) + ": " + response;
		return string();
	}
	const string zak = root.get("token", "").asString();
	if (zak.empty()) err = "ZAK response had no token";
	return zak;
}

static wstring VerifierPath()
{
	char localAppData[MAX_PATH];
	memset(localAppData, 0, sizeof(localAppData));
	GetEnvironmentVariableA("LOCALAPPDATA", localAppData, MAX_PATH);
	const wstring dir = Utf8ToWide(localAppData) + L"\\PKCEAuthDemo";
	CreateDirectoryW(dir.c_str(), nullptr);
	return dir + L"\\verifier.txt";
}

bool SaveVerifierToFile(const string& verifier)
{
	ofstream f(VerifierPath().c_str(), ios::trunc);
	if (!f.is_open()) return false;
	f << verifier;
	return (bool)f;
}

bool LoadVerifierFromFile(string& verifierOut)
{
	ifstream f(VerifierPath().c_str());
	if (!f.is_open()) return false;
	getline(f, verifierOut);
	return !verifierOut.empty();
}
