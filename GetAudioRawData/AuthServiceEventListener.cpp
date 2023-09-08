#include "AuthServiceEventListener.h"
#include <iostream>

using namespace std;

AuthServiceEventListener::AuthServiceEventListener(void(*postToDo)())
{
    postToDo_ = postToDo;
}

void AuthServiceEventListener::onAuthenticationReturn(ZOOM_SDK_NAMESPACE::AuthResult ret) {
    if (ret == ZOOM_SDK_NAMESPACE::AuthResult::AUTHRET_JWTTOKENWRONG)
    {
        // SDK Auth call failed because the JWT token is invalid.
        std::cout << "Auth failed: JWT Token is invalid." << endl;
    }
    else if (ret == ZOOM_SDK_NAMESPACE::AuthResult::AUTHRET_SUCCESS)
    {
        // SDK Authenticated successfully
        std::cout << "Auth succeeded: JWT." << endl;
    }
    else 
        std::cout << "Auth failed: " << ret << endl;
}

void AuthServiceEventListener::onLoginReturnWithReason(LOGINSTATUS ret, IAccountInfo* pAccountInfo, LoginFailReason reason)
{
    std::cout << "onLoginReturnWithReason: " << reason << endl;
    if (postToDo_) postToDo_();
}

void AuthServiceEventListener::onLogout()
{
    std::cout << "onLogout" << endl;
}

void AuthServiceEventListener::onZoomIdentityExpired()
{
    std::cout << "onZoomIdentityExpired" << endl;
}

void AuthServiceEventListener::onZoomAuthIdentityExpired()
{
    std::cout << "onZoomAuthIdentityExpired" << endl;
}

void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status)
{
	std::cout << "onNotificationServiceStatus: " << status << endl;
}

void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status, SDKNotificationServiceError error) {

}


