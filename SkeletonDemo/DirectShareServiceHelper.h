#pragma once

#include "direct_share_helper_interface.h"

using namespace std;
using namespace ZOOMSDK;

class DirectShareServiceHelper :
	public IDirectShareServiceHelper
{

public:


	virtual SDKError SetEvent(IDirectShareServiceHelperEvent* pEvent);


	virtual SDKError CanStartDirectShare();


	virtual bool	 IsDirectShareInProgress();

	virtual SDKError StartDirectShare();

	virtual SDKError StopDirectShare();
};