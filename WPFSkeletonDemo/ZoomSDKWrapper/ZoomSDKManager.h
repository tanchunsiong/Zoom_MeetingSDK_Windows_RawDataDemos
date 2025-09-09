#pragma once

#include <msclr\marshal_cppstd.h>
#include <string>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace ZoomSDKWrapper {

    public enum class SDKError {
        SDKERR_SUCCESS = 0,
        SDKERR_NO_IMPL,
        SDKERR_WRONG_USEAGE,
        SDKERR_INVALID_PARAMETERS,
        SDKERR_LOAD_SDK_DLL_FAILED,
        SDKERR_UNINITIALIZE,
        SDKERR_UNAUTHENTICATION,
        SDKERR_NORECORDINGINPROCESS,
        SDKERR_TRANSCODER_NOFOUND,
        SDKERR_VIDEO_NOTREADY,
        SDKERR_NO_PERMISSION,
        SDKERR_UNKNOWN
    };

    public enum class LoginStatus {
        LOGINRET_NONE = 0,
        LOGINRET_SUCCESS,
        LOGINRET_SDK_NOT_AUTH,
        LOGINRET_AUTH_FAILED,
        LOGINRET_USER_NOT_EXIST,
        LOGINRET_WRONG_PASSWORD,
        LOGINRET_ACCOUNT_LOCKED,
        LOGINRET_ACCOUNT_RESTRICTED,
        LOGINRET_EMAIL_NOT_VERIFIED,
        LOGINRET_TOO_MANY_FAILED_ATTEMPTS,
        LOGINRET_SMS_CODE_WRONG,
        LOGINRET_SMS_CODE_EXPIRED,
        LOGINRET_PHONE_NUMBER_WRONG,
        LOGINRET_UNKNOWN
    };

    public enum class MeetingStatus {
        MEETING_STATUS_IDLE = 0,
        MEETING_STATUS_CONNECTING,
        MEETING_STATUS_WAITINGFORHOST,
        MEETING_STATUS_INMEETING,
        MEETING_STATUS_DISCONNECTING,
        MEETING_STATUS_RECONNECTING,
        MEETING_STATUS_FAILED,
        MEETING_STATUS_ENDED,
        MEETING_STATUS_UNKNOWN
    };

    public ref class AuthEventArgs : EventArgs {
    public:
        property LoginStatus Result;
        property String^ Message;
        
        AuthEventArgs(LoginStatus result, String^ message) {
            Result = result;
            Message = message;
        }
    };

    public ref class MeetingStatusEventArgs : EventArgs {
    public:
        property MeetingStatus Status;
        property String^ Message;
        
        MeetingStatusEventArgs(MeetingStatus status, String^ message) {
            Status = status;
            Message = message;
        }
    };

    public ref class UserEventArgs : EventArgs {
    public:
        property unsigned int UserId;
        property String^ UserName;
        property bool IsHost;
        
        UserEventArgs(unsigned int userId, String^ userName, bool isHost) {
            UserId = userId;
            UserName = userName;
            IsHost = isHost;
        }
    };

    public ref class ZoomSDKManager {
    private:
        void* m_pSDK;
        void* m_pAuthService;
        void* m_pMeetingService;
        bool m_bInitialized;
        bool m_bAuthenticated;
        
        // Native callback handlers
        void* m_pAuthHandler;
        void* m_pMeetingHandler;

    public:
        // Events
        event EventHandler<AuthEventArgs^>^ AuthenticationCompleted;
        event EventHandler<MeetingStatusEventArgs^>^ MeetingStatusChanged;
        event EventHandler<UserEventArgs^>^ UserJoined;
        event EventHandler<UserEventArgs^>^ UserLeft;
        event EventHandler^ MeetingEnded;

        ZoomSDKManager();
        ~ZoomSDKManager();
        !ZoomSDKManager();

        // SDK Management
        SDKError Initialize();
        void Cleanup();

        // Authentication
        SDKError Authenticate(String^ jwtToken);
        void Logout();

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
        property MeetingStatus CurrentMeetingStatus { MeetingStatus get(); }

        // Public event handlers for native callbacks
        void OnAuthenticationResult(LoginStatus result, String^ message);
        void OnMeetingStatusChanged(MeetingStatus status, String^ message);
        void OnUserJoined(unsigned int userId, String^ userName, bool isHost);
        void OnUserLeft(unsigned int userId);
        void OnMeetingEnded();

    private:
        // Helper methods
        String^ ConvertToManagedString(const wchar_t* nativeString);
        std::wstring ConvertToNativeString(String^ managedString);
        void InitializeCallbacks();
        void CleanupCallbacks();
    };
}
