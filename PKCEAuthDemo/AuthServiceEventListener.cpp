#include "AuthServiceEventListener.h"
#include <iostream>

using namespace std;

AuthServiceEventListener::AuthServiceEventListener(void(*postToDo)())
{
	postToDo_ = postToDo;
}

void AuthServiceEventListener::onAuthenticationReturn(ZOOM_SDK_NAMESPACE::AuthResult ret)
{
	if (ret == ZOOM_SDK_NAMESPACE::AUTHRET_SUCCESS)
	{
		cout << "SDK auth succeeded." << endl;
		if (postToDo_) postToDo_();
	}
	else
	{
		cout << "SDK auth failed: " << (int)ret << endl;
	}
}

void AuthServiceEventListener::onLoginReturnWithReason(LOGINSTATUS ret, IAccountInfo* pAccountInfo, LoginFailReason reason)
{
	cout << "onLoginReturnWithReason: " << reason << endl;
}

void AuthServiceEventListener::onLogout()
{
	cout << "onLogout" << endl;
}

void AuthServiceEventListener::onZoomIdentityExpired()
{
	cout << "onZoomIdentityExpired" << endl;
}

void AuthServiceEventListener::onZoomAuthIdentityExpired()
{
	cout << "onZoomAuthIdentityExpired" << endl;
}

void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status)
{
	cout << "onNotificationServiceStatus: " << status << endl;
}

void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status, SDKNotificationServiceError error)
{
	cout << "onNotificationServiceStatus: " << status << ", err: " << (int)error << endl;
}
