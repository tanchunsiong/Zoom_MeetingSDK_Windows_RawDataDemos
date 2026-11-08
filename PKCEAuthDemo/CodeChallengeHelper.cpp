#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>

#include "CodeChallengeHelper.h"

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

using namespace std;

string CodeChallengeHelper::verifier;

static void strFindAndReplace(string& str, const string& oldStr, const string& newStr)
{
	string::size_type pos = 0u;
	while ((pos = str.find(oldStr, pos)) != string::npos)
	{
		str.replace(pos, oldStr.length(), newStr);
		pos += newStr.length();
	}
}

static inline void trimPadding(string& s)
{
	s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return ch != '=';
	}).base(), s.end());
}

// base64url (RFC 4648 section 5) without padding, as required for the PKCE
// verifier and the S256 challenge value.
static string Base64UrlNoPad(const unsigned char* data, unsigned long dataLen)
{
	DWORD base64Len = 0;
	if (!CryptBinaryToStringA(data, dataLen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &base64Len))
	{
		return string();
	}
	vector<char> base64(base64Len + 1);
	if (!CryptBinaryToStringA(data, dataLen, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, base64.data(), &base64Len))
	{
		return string();
	}
	string out(base64.data(), base64Len);
	strFindAndReplace(out, "+", "-");
	strFindAndReplace(out, "/", "_");
	trimPadding(out);
	return out;
}

void CodeChallengeHelper::createCodeVerifier()
{
	unsigned char raw[32] = {}; // 32-byte verifier, per the official guide

	// Crypto-strong generator; rand() fallback only if CNG is unavailable.
	if (BCryptGenRandom(nullptr, raw, (ULONG)sizeof(raw), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
	{
		srand((unsigned)time(nullptr));
		for (size_t i = 0; i < sizeof(raw); ++i)
		{
			raw[i] = (unsigned char)(rand() & 0xFF);
		}
	}

	verifier = Base64UrlNoPad(raw, (ULONG)sizeof(raw));
}

string CodeChallengeHelper::getCodeChallenge()
{
	if (verifier.empty())
	{
		return string();
	}

	BCRYPT_ALG_HANDLE hAlg = nullptr;
	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0)
	{
		return string();
	}

	string result;

	ULONG hashObjLen = 0;
	DWORD cb = sizeof(hashObjLen);
	ULONG hashLen = 0;
	if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PUCHAR)&hashLen, sizeof(hashLen), &cb, 0) == 0 && hashLen > 0 &&
		BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&hashObjLen, sizeof(hashObjLen), &cb, 0) == 0 && hashObjLen > 0)
	{
		vector<unsigned char> hash(hashLen);
		vector<unsigned char> hashObj(hashObjLen);

		BCRYPT_HASH_HANDLE hHash = nullptr;
		if (BCryptCreateHash(hAlg, &hHash, hashObj.data(), hashObjLen, nullptr, 0, 0) == 0)
		{
			BCryptHashData(hHash, (PUCHAR)verifier.c_str(), (ULONG)verifier.size(), 0);
			if (BCryptFinishHash(hHash, hash.data(), hashLen, 0) == 0)
			{
				result = Base64UrlNoPad(hash.data(), hashLen);
			}
			BCryptDestroyHash(hHash);
		}
	}

	BCryptCloseAlgorithmProvider(hAlg, 0);
	return result;
}
