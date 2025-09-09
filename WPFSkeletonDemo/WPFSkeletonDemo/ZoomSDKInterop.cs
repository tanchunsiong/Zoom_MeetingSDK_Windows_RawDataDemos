using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace WPFSkeletonDemo
{
    /// <summary>
    /// Interop helper class for Zoom Meeting SDK
    /// Provides a clean .NET interface to the native ZoomSDKWrapper.dll
    /// </summary>
    public static class ZoomSDKInterop
    {
        #region DLL Loading

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool FreeLibrary(IntPtr hModule);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern uint GetLastError();

        private static IntPtr _zoomSDKWrapperHandle = IntPtr.Zero;
        private static bool _dllLoaded = false;

        /// <summary>
        /// Explicitly load the ZoomSDKWrapper.dll and its dependencies
        /// </summary>
        /// <returns>True if successful, false otherwise</returns>
        public static bool LoadZoomSDKWrapper()
        {
            if (_dllLoaded)
                return true;

            try
            {
                // Get the application directory
                string appDir = AppDomain.CurrentDomain.BaseDirectory;
                string zoomSDKWrapperPath = Path.Combine(appDir, "ZoomSDKWrapper.dll");

                // Check if the DLL exists
                if (!File.Exists(zoomSDKWrapperPath))
                {
                    throw new FileNotFoundException($"ZoomSDKWrapper.dll not found at: {zoomSDKWrapperPath}");
                }

                // Pre-load critical Zoom SDK dependencies in order
                string[] criticalDlls = {
                    "sdk.dll",
                    "zoombase_crypto_shared.dll",
                    "util.dll"
                };

                foreach (string dll in criticalDlls)
                {
                    string dllPath = Path.Combine(appDir, dll);
                    if (File.Exists(dllPath))
                    {
                        IntPtr handle = LoadLibrary(dllPath);
                        if (handle == IntPtr.Zero)
                        {
                            uint error = GetLastError();
                            throw new Exception($"Failed to load {dll}. Error code: {error} (0x{error:X})");
                        }
                    }
                }

                // Load the main wrapper DLL
                _zoomSDKWrapperHandle = LoadLibrary(zoomSDKWrapperPath);
                if (_zoomSDKWrapperHandle == IntPtr.Zero)
                {
                    uint error = GetLastError();
                    throw new Exception($"Failed to load ZoomSDKWrapper.dll. Error code: {error} (0x{error:X})");
                }

                _dllLoaded = true;
                return true;
            }
            catch (Exception ex)
            {
                throw new Exception($"Failed to load ZoomSDKWrapper.dll: {ex.Message}", ex);
            }
        }

        /// <summary>
        /// Unload the ZoomSDKWrapper.dll
        /// </summary>
        public static void UnloadZoomSDKWrapper()
        {
            if (_zoomSDKWrapperHandle != IntPtr.Zero)
            {
                FreeLibrary(_zoomSDKWrapperHandle);
                _zoomSDKWrapperHandle = IntPtr.Zero;
                _dllLoaded = false;
            }
        }

        #endregion

        #region P/Invoke Declarations

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeSDK();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void CleanupSDK();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Authenticate(string jwtToken, AuthResultCallback callback);

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int JoinMeeting(ulong meetingNumber, string passcode, string userName,
            MeetingStatusCallback statusCallback, UserJoinCallback userJoinCallback,
            UserLeaveCallback userLeaveCallback, MeetingEndCallback meetingEndCallback);

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int StartMeeting(ulong meetingNumber, string userName,
            MeetingStatusCallback statusCallback, UserJoinCallback userJoinCallback,
            UserLeaveCallback userLeaveCallback, MeetingEndCallback meetingEndCallback);

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int LeaveMeetingNative();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int EndMeetingNative();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetMeetingIDNative();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetMeetingPasswordNative();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetParticipantCountNative();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int MuteAudioNative(bool mute);

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int MuteVideoNative(bool mute);

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool IsAudioMutedNative();

        [DllImport("ZoomSDKWrapper.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool IsVideoMutedNative();

        #endregion

        #region Callback Delegate Types

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void AuthResultCallback(int result);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void MeetingStatusCallback(int status, IntPtr message);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void UserJoinCallback(uint userId, IntPtr userName, bool isHost);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void UserLeaveCallback(uint userId);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void MeetingEndCallback();

        #endregion

        #region Events

        public static event Action<int>? AuthenticationCompleted;
        public static event Action<int, string>? MeetingStatusChanged;
        public static event Action<uint, string, bool>? UserJoined;
        public static event Action<uint>? UserLeft;
        public static event Action? MeetingEnded;

        #endregion

        #region Callback Implementations

        private static readonly AuthResultCallback _authCallback = OnAuthResult;
        private static readonly MeetingStatusCallback _meetingStatusCallback = OnMeetingStatus;
        private static readonly UserJoinCallback _userJoinCallback = OnUserJoin;
        private static readonly UserLeaveCallback _userLeaveCallback = OnUserLeave;
        private static readonly MeetingEndCallback _meetingEndCallback = OnMeetingEnd;

        private static void OnAuthResult(int result)
        {
            AuthenticationCompleted?.Invoke(result);
        }

        private static void OnMeetingStatus(int status, IntPtr messagePtr)
        {
            string message = Marshal.PtrToStringAnsi(messagePtr) ?? "Unknown";
            MeetingStatusChanged?.Invoke(status, message);
        }

        private static void OnUserJoin(uint userId, IntPtr userNamePtr, bool isHost)
        {
            string userName = Marshal.PtrToStringAnsi(userNamePtr) ?? "Unknown";
            UserJoined?.Invoke(userId, userName, isHost);
        }

        private static void OnUserLeave(uint userId)
        {
            UserLeft?.Invoke(userId);
        }

        private static void OnMeetingEnd()
        {
            MeetingEnded?.Invoke();
        }

        #endregion

        #region Public API Methods

        /// <summary>
        /// Initialize the Zoom Meeting SDK
        /// </summary>
        /// <returns>0 on success, error code otherwise</returns>
        public static int Initialize()
        {
            try
            {
                // First, try to pre-load dependencies manually
                LoadZoomSDKWrapper();
                
                // Then call the actual initialization
                return InitializeSDK();
            }
            catch (DllNotFoundException ex)
            {
                throw new Exception($"ZoomSDKWrapper.dll or its dependencies could not be found: {ex.Message}", ex);
            }
            catch (BadImageFormatException ex)
            {
                throw new Exception($"ZoomSDKWrapper.dll has incorrect format (architecture mismatch): {ex.Message}", ex);
            }
            catch (Exception ex)
            {
                throw new Exception($"Failed to initialize Zoom SDK: {ex.Message}", ex);
            }
        }

        /// <summary>
        /// Clean up the Zoom Meeting SDK
        /// </summary>
        public static void Cleanup()
        {
            try
            {
                CleanupSDK();
            }
            finally
            {
                UnloadZoomSDKWrapper();
            }
        }

        /// <summary>
        /// Authenticate with Zoom using JWT token
        /// </summary>
        /// <param name="jwtToken">JWT token for authentication</param>
        /// <returns>0 on success, error code otherwise</returns>
        public static int AuthenticateWithToken(string jwtToken)
        {
            return Authenticate(jwtToken, _authCallback);
        }

        /// <summary>
        /// Join a Zoom meeting
        /// </summary>
        /// <param name="meetingNumber">Meeting number</param>
        /// <param name="passcode">Meeting passcode</param>
        /// <param name="userName">Display name</param>
        /// <returns>0 on success, error code otherwise</returns>
        public static int JoinMeeting(ulong meetingNumber, string passcode, string userName)
        {
            return JoinMeeting(meetingNumber, passcode, userName,
                _meetingStatusCallback, _userJoinCallback, _userLeaveCallback, _meetingEndCallback);
        }

        /// <summary>
        /// Start a Zoom meeting
        /// </summary>
        /// <param name="meetingNumber">Meeting number</param>
        /// <param name="userName">Display name</param>
        /// <returns>0 on success, error code otherwise</returns>
        public static int StartMeeting(ulong meetingNumber, string userName)
        {
            return StartMeeting(meetingNumber, userName,
                _meetingStatusCallback, _userJoinCallback, _userLeaveCallback, _meetingEndCallback);
        }

        /// <summary>
        /// Leave the current meeting
        /// </summary>
        /// <returns>0 on success, error code otherwise</returns>
        public static int LeaveMeeting()
        {
            return LeaveMeetingNative();
        }

        /// <summary>
        /// End the current meeting
        /// </summary>
        /// <returns>0 on success, error code otherwise</returns>
        public static int EndMeeting()
        {
            return EndMeetingNative();
        }

        /// <summary>
        /// Get the current meeting ID
        /// </summary>
        /// <returns>Meeting ID</returns>
        public static int GetMeetingID()
        {
            return GetMeetingIDNative();
        }

        /// <summary>
        /// Get the meeting password
        /// </summary>
        /// <returns>Meeting password string</returns>
        public static string GetMeetingPassword()
        {
            IntPtr ptr = GetMeetingPasswordNative();
            return Marshal.PtrToStringAnsi(ptr) ?? "";
        }

        /// <summary>
        /// Get the number of participants in the meeting
        /// </summary>
        /// <returns>Participant count</returns>
        public static int GetParticipantCount()
        {
            return GetParticipantCountNative();
        }

        /// <summary>
        /// Mute or unmute audio
        /// </summary>
        /// <param name="mute">True to mute, false to unmute</param>
        /// <returns>0 on success, error code otherwise</returns>
        public static int MuteAudio(bool mute)
        {
            return MuteAudioNative(mute);
        }

        /// <summary>
        /// Mute or unmute video
        /// </summary>
        /// <param name="mute">True to mute, false to unmute</param>
        /// <returns>0 on success, error code otherwise</returns>
        public static int MuteVideo(bool mute)
        {
            return MuteVideoNative(mute);
        }

        /// <summary>
        /// Check if audio is muted
        /// </summary>
        /// <returns>True if muted, false otherwise</returns>
        public static bool IsAudioMuted()
        {
            return IsAudioMutedNative();
        }

        /// <summary>
        /// Check if video is muted
        /// </summary>
        /// <returns>True if muted, false otherwise</returns>
        public static bool IsVideoMuted()
        {
            return IsVideoMutedNative();
        }

        #endregion
    }
}
