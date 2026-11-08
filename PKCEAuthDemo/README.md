# PKCEAuthDemo

Meeting SDK for Windows demo that authenticates with OAuth Authorization Code + PKCE using a Zoom public client - no JWT and no SDK key/secret anywhere in the flow.

**Status: verified end to end live.** PKCE authorize -> token exchange -> ZAK -> SDKAuth(publicAppKey) -> Start(SDK_UT_WITHOUT_LOGIN) all work.

Based on the official guide: https://developers.zoom.us/docs/meeting-sdk/windows/start-join-mtg-webinar/pkce/

## Client id: use the PKCE public client id

The app must use the client id issued when **public client OAuth / PKCE** is enabled on the Marketplace app (credentials.pkcePrivateClient.clientId). Do NOT use the classic dev client id: it is a confidential client and rejects header-less token exchanges with invalid_client. The production client id does not work until the app is published.

The SDK auth uses the same id: AuthContext.publicAppKey = the PKCE client id. No JWT, no SDK key/secret.

## Redirect URI rule

redirect_uri must be byte-identical - including any query string - between the authorize request and the token exchange. The demo guarantees this by deriving both from the same config value (EffectiveRedirectUri).

## Callback modes

- manual (default, verified): opens the Zoom authorize page; paste the code= value (or the whole redirect URL) into the console.
- loopback: set "callback_mode": "loopback" and the app listens on http://127.0.0.1:<loopback_port>/callback, capturing the code automatically - no pasting. For this, the redirect must actually go to loopback: add http://127.0.0.1:<loopback_port>/callback (exact, including any path) to the allow list on the Marketplace app. In this mode the redirect_uri config value is not used for the browser redirect; the loopback URL is used instead, and it is also what the token exchange sends, keeping the byte-identical rule intact.

## Config keys (config.json, copy from config-example.json)

- public_client_id - the PKCE public client id (see above). Placeholder in the example.
- redirect_uri - must match the app registration exactly, query string included.
- callback_mode - manual | loopback.
- loopback_port - port for the loopback listener (callback_mode=loopback only).
- meeting_number - meeting to start; placeholder in the example, set the real value.
- user_name - display name when starting.
- video_off / audio_off - start with camera/mic off.
- passcode - parsed for future use; StartParam4WithoutLogin in SDK 7.0.2 has no passcode member (userZAK, userName, zoomuserType, meetingNumber, vanityID, customer_key, AV flags only). Starting via meetingNumber + userZAK as the host does not prompt for a passcode.

No secrets belong in this repo: config.json holds ids and non-secret settings only; the token exchange is header-less (client_id in the form body).

## Run

1. Copy config-example.json to config.json next to the exe (x64\Debug\ after build) and fill in real values.
2. PKCEAuthDemo.exe (no args): prints the authorize URL, opens the browser, and waits:
   - manual mode: paste the code= value or the entire redirected URL.
   - loopback mode: capture is automatic.
3. The same process then exchanges code+verifier for tokens, fetches the ZAK (GET https://api.zoom.us/v2/users/me/token?type=zak), authenticates the SDK with publicAppKey, and starts the meeting on AUTHRET_SUCCESS.

argv mode: PKCEAuthDemo.exe "<callback url>" resumes from the code-capture step using the verifier saved in %LOCALAPPDATA%\PKCEAuthDemo\verifier.txt.

## TLS note

The exe links the Zoom SDK redist libcurl (OpenSSL); a CA bundle named curl-ca-bundle.crt must sit next to the exe (copied on build). Without it the token request fails SSL verification.
