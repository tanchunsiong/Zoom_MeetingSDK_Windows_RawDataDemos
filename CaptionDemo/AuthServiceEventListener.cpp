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
        std::cout << "Auth failed: JWT Token is invalid." << std::endl;
    }
    else if (ret == ZOOM_SDK_NAMESPACE::AuthResult::AUTHRET_SUCCESS)
    {
        // SDK Authenticated successfully
        std::cout << "Auth succeeded: JWT." << std::endl;
    }
    else 
        std::cout << "Auth failed: " << ret << std::endl;
}

void AuthServiceEventListener::onLoginReturnWithReason(LOGINSTATUS ret, IAccountInfo* pAccountInfo, LoginFailReason reason)
{
    std::cout << "onLoginReturnWithReason: " << reason << std::endl;
    if (postToDo_) postToDo_();
}

void AuthServiceEventListener::onLogout()
{
    std::cout << "onLogout" << std::endl;
}

void AuthServiceEventListener::onZoomIdentityExpired()
{
    std::cout << "onZoomIdentityExpired" << std::endl;
}

void AuthServiceEventListener::onZoomAuthIdentityExpired()
{
    std::cout << "onZoomAuthIdentityExpired" << std::endl;
}

void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status)
{
    std::cout << "onNotificationServiceStatus: " << status << std::endl;
}

void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status, SDKNotificationServiceError error) {

}

