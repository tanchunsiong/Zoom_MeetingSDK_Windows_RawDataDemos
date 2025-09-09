#include "ZoomSDKWrapper.h"
#include "zoom_sdk.h"
#include "auth_service_interface.h"
#include "meeting_service_interface.h"
#include "meeting_service_components/meeting_audio_interface.h"
#include "meeting_service_components/meeting_video_interface.h"
#include "meeting_service_components/meeting_participants_ctrl_interface.h"
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;
using namespace ZOOMSDK;

namespace ZoomSDKWrapper {

    // Constructor
    ZoomSDKManager::ZoomSDKManager() {
        m_pAuthService = nullptr;
        m_pMeetingService = nullptr;
        m_bInitialized = false;
        m_bAuthenticated = false;
    }

    // Destructor
    ZoomSDKManager::~ZoomSDKManager() {
        this->!ZoomSDKManager();
    }

    // Finalizer
    ZoomSDKManager::!ZoomSDKManager() {
        Cleanup();
    }

    // Initialize SDK
    SDKError ZoomSDKManager::Initialize() {
        try {
            if (m_bInitialized) {
                return SDKError::SDKERR_SUCCESS;
            }

            ZOOMSDK::InitParam initParam;
            initParam.strWebDomain = L"https://zoom.us";
            initParam.strBrandingName = nullptr;
            initParam.strSupportUrl = nullptr;
            initParam.emLanguageID = ZOOMSDK::LANGUAGE_English;
            initParam.enableGenerateDump = false;
            initParam.enableLogByDefault = true;
            initParam.uiLogFileSize = 5;

            ZOOMSDK::SDKError result = ZOOMSDK::InitSDK(initParam);
            if (result == ZOOMSDK::SDKERR_SUCCESS) {
                // Create services using temporary native pointers
                ZOOMSDK::IAuthService* pAuthService = nullptr;
                ZOOMSDK::IMeetingService* pMeetingService = nullptr;
                
                ZOOMSDK::CreateAuthService(&pAuthService);
                ZOOMSDK::CreateMeetingService(&pMeetingService);
                
                m_pAuthService = pAuthService;
                m_pMeetingService = pMeetingService;
                m_bInitialized = true;
            }

            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Cleanup SDK
    void ZoomSDKManager::Cleanup() {
        try {
            if (m_bInitialized) {
                if (m_pMeetingService) {
                    ZOOMSDK::DestroyMeetingService(static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService));
                    m_pMeetingService = nullptr;
                }
                if (m_pAuthService) {
                    ZOOMSDK::DestroyAuthService(static_cast<ZOOMSDK::IAuthService*>(m_pAuthService));
                    m_pAuthService = nullptr;
                }
                ZOOMSDK::CleanUPSDK();
                m_bInitialized = false;
                m_bAuthenticated = false;
            }
        }
        catch (...) {
            // Ignore cleanup errors
        }
    }

    // Authenticate
    SDKError ZoomSDKManager::Authenticate(String^ jwtToken) {
        try {
            if (!m_bInitialized || !m_pAuthService) {
                return SDKError::SDKERR_UNINITIALIZE;
            }

            ZOOMSDK::IAuthService* pAuthService = static_cast<ZOOMSDK::IAuthService*>(m_pAuthService);
            std::wstring nativeToken = ConvertToNativeString(jwtToken);

            ZOOMSDK::AuthContext authContext;
            authContext.jwt_token = nativeToken.c_str();

            ZOOMSDK::SDKError result = pAuthService->SDKAuth(authContext);
            if (result == ZOOMSDK::SDKERR_SUCCESS) {
                m_bAuthenticated = true;
            }

            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Join Meeting
    SDKError ZoomSDKManager::JoinMeeting(unsigned long long meetingNumber, String^ passcode, String^ userName) {
        try {
            if (!m_bInitialized || !m_bAuthenticated || !m_pMeetingService) {
                return SDKError::SDKERR_UNAUTHENTICATION;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            
            ZOOMSDK::JoinParam joinParam;
            joinParam.userType = ZOOMSDK::SDK_UT_WITHOUT_LOGIN;
            
            ZOOMSDK::JoinParam4WithoutLogin& withoutLoginParam = joinParam.param.withoutloginuserJoin;
            withoutLoginParam.meetingNumber = meetingNumber;
            withoutLoginParam.userName = ConvertToNativeString(userName).c_str();
            withoutLoginParam.psw = ConvertToNativeString(passcode).c_str();

            ZOOMSDK::SDKError result = pMeetingService->Join(joinParam);
            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Start Meeting
    SDKError ZoomSDKManager::StartMeeting(unsigned long long meetingNumber, String^ userName) {
        try {
            if (!m_bInitialized || !m_bAuthenticated || !m_pMeetingService) {
                return SDKError::SDKERR_UNAUTHENTICATION;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            
            ZOOMSDK::StartParam startParam;
            startParam.userType = ZOOMSDK::SDK_UT_WITHOUT_LOGIN;
            
            ZOOMSDK::StartParam4WithoutLogin& withoutLoginParam = startParam.param.withoutloginStart;
            withoutLoginParam.meetingNumber = meetingNumber;
            withoutLoginParam.userName = ConvertToNativeString(userName).c_str();
            withoutLoginParam.userZAK = L"";

            ZOOMSDK::SDKError result = pMeetingService->Start(startParam);
            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Leave Meeting
    SDKError ZoomSDKManager::LeaveMeeting() {
        try {
            if (!m_pMeetingService) {
                return SDKError::SDKERR_UNINITIALIZE;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::SDKError result = pMeetingService->Leave(ZOOMSDK::LEAVE_MEETING);
            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // End Meeting
    SDKError ZoomSDKManager::EndMeeting() {
        try {
            if (!m_pMeetingService) {
                return SDKError::SDKERR_UNINITIALIZE;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::SDKError result = pMeetingService->Leave(ZOOMSDK::END_MEETING);
            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Mute Audio
    SDKError ZoomSDKManager::MuteAudio(bool mute) {
        try {
            if (!m_pMeetingService) {
                return SDKError::SDKERR_UNINITIALIZE;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingAudioController* pAudioController = pMeetingService->GetMeetingAudioController();
            if (!pAudioController) {
                return SDKError::SDKERR_NO_IMPL;
            }

            ZOOMSDK::SDKError result;
            if (mute) {
                result = pAudioController->MuteAudio(0, true); // 0 means current user
            } else {
                result = pAudioController->UnMuteAudio(0); // 0 means current user
            }
            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Is Audio Muted
    bool ZoomSDKManager::IsAudioMuted() {
        try {
            if (!m_pMeetingService) {
                return false;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingAudioController* pAudioController = pMeetingService->GetMeetingAudioController();
            if (!pAudioController) {
                return false;
            }

            // We need to get the current user's audio status
            // For simplicity, we'll return false here since there's no direct IsAudioMuted method
            // In a real implementation, you'd need to track the audio status through events
            return false;
        }
        catch (...) {
            return false;
        }
    }

    // Mute Video
    SDKError ZoomSDKManager::MuteVideo(bool mute) {
        try {
            if (!m_pMeetingService) {
                return SDKError::SDKERR_UNINITIALIZE;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingVideoController* pVideoController = pMeetingService->GetMeetingVideoController();
            if (!pVideoController) {
                return SDKError::SDKERR_NO_IMPL;
            }

            ZOOMSDK::SDKError result;
            if (mute) {
                result = pVideoController->MuteVideo();
            } else {
                result = pVideoController->UnmuteVideo();
            }
            return static_cast<SDKError>(result);
        }
        catch (...) {
            return SDKError::SDKERR_UNKNOWN;
        }
    }

    // Is Video Muted
    bool ZoomSDKManager::IsVideoMuted() {
        try {
            if (!m_pMeetingService) {
                return false;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingVideoController* pVideoController = pMeetingService->GetMeetingVideoController();
            if (!pVideoController) {
                return false;
            }

            // We need to get the current user's video status
            // For simplicity, we'll return false here since there's no direct IsVideoMuted method
            // In a real implementation, you'd need to track the video status through events
            return false;
        }
        catch (...) {
            return false;
        }
    }

    // Get Meeting ID
    unsigned long long ZoomSDKManager::GetMeetingID() {
        try {
            if (!m_pMeetingService) {
                return 0;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingInfo* pMeetingInfo = pMeetingService->GetMeetingInfo();
            if (!pMeetingInfo) {
                return 0;
            }

            return pMeetingInfo->GetMeetingNumber();
        }
        catch (...) {
            return 0;
        }
    }

    // Get Meeting Password
    String^ ZoomSDKManager::GetMeetingPassword() {
        try {
            if (!m_pMeetingService) {
                return "";
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingInfo* pMeetingInfo = pMeetingService->GetMeetingInfo();
            if (!pMeetingInfo) {
                return "";
            }

            const zchar_t* password = pMeetingInfo->GetMeetingPassword();
            return ConvertToManagedString(password);
        }
        catch (...) {
            return "";
        }
    }

    // Get Participant Count
    int ZoomSDKManager::GetParticipantCount() {
        try {
            if (!m_pMeetingService) {
                return 0;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            ZOOMSDK::IMeetingParticipantsController* pParticipantsController = pMeetingService->GetMeetingParticipantsController();
            if (!pParticipantsController) {
                return 0;
            }

            ZOOMSDK::IList<unsigned int>* pUserList = pParticipantsController->GetParticipantsList();
            return pUserList ? pUserList->GetCount() : 0;
        }
        catch (...) {
            return 0;
        }
    }

    // Properties
    bool ZoomSDKManager::IsInitialized::get() {
        return m_bInitialized;
    }

    bool ZoomSDKManager::IsAuthenticated::get() {
        return m_bAuthenticated;
    }

    bool ZoomSDKManager::IsInMeeting::get() {
        try {
            if (!m_pMeetingService) {
                return false;
            }

            ZOOMSDK::IMeetingService* pMeetingService = static_cast<ZOOMSDK::IMeetingService*>(m_pMeetingService);
            return pMeetingService->GetMeetingStatus() == ZOOMSDK::MEETING_STATUS_INMEETING;
        }
        catch (...) {
            return false;
        }
    }

    // Helper Methods
    String^ ZoomSDKManager::ConvertToManagedString(const zchar_t* nativeString) {
        if (!nativeString) {
            return "";
        }
        return gcnew String(nativeString);
    }

    std::wstring ZoomSDKManager::ConvertToNativeString(String^ managedString) {
        if (!managedString) {
            return L"";
        }
        marshal_context ctx;
        return ctx.marshal_as<std::wstring>(managedString);
    }
}
