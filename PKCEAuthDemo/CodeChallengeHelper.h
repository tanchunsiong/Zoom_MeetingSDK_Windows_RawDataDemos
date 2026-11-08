#pragma once

#include <string>

// PKCE code verifier / challenge per the official Meeting SDK for Windows
// PKCE guide: 32 random bytes, base64url-encoded without padding;
// challenge = base64url(SHA-256(verifier)), method S256.
class CodeChallengeHelper
{
public:
	static std::string verifier;
	static void createCodeVerifier();
	static std::string getCodeChallenge();
};
