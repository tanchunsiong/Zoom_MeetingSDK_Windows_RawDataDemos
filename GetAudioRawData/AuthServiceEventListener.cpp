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
        cout << "Auth failed: JWT Token is invalid." << endl;
    }
    else if (ret == ZOOM_SDK_NAMESPACE::AuthResult::AUTHRET_SUCCESS)
    {
        // SDK Authenticated successfully
        cout << "Auth succeeded: JWT." << endl;
    }
    else 
        cout << "Auth failed: " << ret << endl;
}

void AuthServiceEventListener::onLoginReturnWithReason(LOGINSTATUS ret, IAccountInfo* pAccountInfo, LoginFailReason reason)
{
    cout << "onLoginReturnWithReason: " << reason << endl;
    if (postToDo_) postToDo_();
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
