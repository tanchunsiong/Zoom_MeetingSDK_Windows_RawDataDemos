#pragma once

#include <msclr\marshal_cppstd.h>
#include <string>

using namespace System;

// Forward declaration for zchar_t
#if defined(WIN32)
typedef wchar_t zchar_t;
#else
typedef char zchar_t;
#endif

namespace ZoomSDKWrapper {

    public enum class SDKError {
        SDKERR_SUCCESS = 0,
        SDKERR_NO_IMPL,
        SDKERR_WRONG_USEAGE,
        SDKERR_INVALID_PARAMETERS,
        SDKERR_LOAD_SDK_DLL_FAILED,
        SDKERR_UNINITIALIZE,
        SDKERR_UNAUTHENTICATION,
        SDKERR_UNKNOWN
    };

    public enum class LoginStatus {
        LOGINRET_NONE = 0,
        LOGINRET_SUCCESS,
        LOGINRET_SDK_NOT_AUTH,
        LOGINRET_AUTH_FAILED,
        LOGINRET_UNKNOWN
    };

    public enum class MeetingStatus {
        MEETING_STATUS_IDLE = 0,
        MEETING_STATUS_CONNECTING,
        MEETING_STATUS_INMEETING,
        MEETING_STATUS_DISCONNECTING,
        MEETING_STATUS_FAILED,
        MEETING_STATUS_ENDED,
        MEETING_STATUS_UNKNOWN
    };

    public ref class ZoomSDKManager {
    private:
        void* m_pAuthService;
        void* m_pMeetingService;
        bool m_bInitialized;
        bool m_bAuthenticated;

    public:
        ZoomSDKManager();
        ~ZoomSDKManager();
        !ZoomSDKManager();

        // SDK Management
        SDKError Initialize();
        void Cleanup();

        // Authentication
        SDKError Authenticate(String^ jwtToken);

        // Meeting Management
        SDKError JoinMeeting(unsigned long long meetingNumber, String^ passcode, String^ userName);
        SDKError StartMeeting(unsigned long long meetingNumber, String^ userName);
        SDKError LeaveMeeting();
        SDKError EndMeeting();

        // Audio Controls
        SDKError MuteAudio(bool mute);
        bool IsAudioMuted();

        // Video Controls
        SDKError MuteVideo(bool mute);
        bool IsVideoMuted();

        // Meeting Info
        unsigned long long GetMeetingID();
        String^ GetMeetingPassword();
        int GetParticipantCount();

        // Properties
        property bool IsInitialized { bool get(); }
        property bool IsAuthenticated { bool get(); }
        property bool IsInMeeting { bool get(); }

    private:
        // Helper methods
        String^ ConvertToManagedString(const zchar_t* nativeString);
        std::wstring ConvertToNativeString(String^ managedString);
    };
}
