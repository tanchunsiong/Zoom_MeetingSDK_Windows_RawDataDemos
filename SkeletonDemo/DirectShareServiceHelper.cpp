#include "windows.h"
#include "zoom_sdk_def.h" 

#include "DirectShareServiceHelper.h"

using namespace std;
using namespace ZOOM_SDK_NAMESPACE;

SDKError DirectShareServiceHelper::SetEvent(IDirectShareServiceHelperEvent* pEvent)
{
	return SDKError();
}

SDKError DirectShareServiceHelper::CanStartDirectShare()
{
	return SDKError();
}

bool DirectShareServiceHelper::IsDirectShareInProgress()
{
	return false;
}

SDKError DirectShareServiceHelper::StartDirectShare()
{
	return SDKError();
}

SDKError DirectShareServiceHelper::StopDirectShare()
{
	return SDKError();
}
