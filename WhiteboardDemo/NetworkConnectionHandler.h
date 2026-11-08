#pragma once
#include <windows.h>
#include <zoom_sdk.h>
#include <network_connection_handler_interface.h>

using namespace ZOOMSDK;

class NetworkConnectionHandler :
    public INetworkConnectionHandler
{
	void (*postToDo_)();
public:
	NetworkConnectionHandler(void (*postToDo)());

	/// \brief Notification callback of completing the proxy detection.
	virtual void onProxyDetectComplete();
	/// \brief The callback will be triggered if the proxy requests to input the username and password.
	/// \remarks Use the handler to configure the related information. For more details, see \link IProxySettingHandler \endlink. 
	///The handler will be destroyed once the function calls end.
	virtual void onProxySettingNotification(IProxySettingHandler* handler);

	/// \brief The callback will be triggered when the SSL is verified.
	/// \remarks Use the handler to check the related information. For more details, see \link ISSLCertVerificationHandler \endlink.
	///The handler will be destroyed once the function calls end.
	virtual void onSSLCertVerifyNotification(ISSLCertVerificationHandler* handler);
};

