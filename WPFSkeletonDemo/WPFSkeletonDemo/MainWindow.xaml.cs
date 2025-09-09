using System;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Windows;
using ZoomSDKWrapper;

namespace WPFSkeletonDemo
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // Data classes
        public class UserInfo
        {
            public uint UserId { get; set; }
            public string UserName { get; set; } = "";
            public bool IsHost { get; set; }

            public override string ToString()
            {
                return $"{UserName} (ID: {UserId}, Host: {IsHost})";
            }
        }

        private ObservableCollection<UserInfo> _participants;
        private ZoomSDKManager _zoomSDK;

        public MainWindow()
        {
            InitializeComponent();
            _participants = new ObservableCollection<UserInfo>();
            lstParticipants.ItemsSource = _participants;

            // Initialize log first
            txtLog.Text = "Application started. Loading configuration...\r\n";

            // Initialize Zoom SDK Manager
            _zoomSDK = new ZoomSDKManager();

            // Load configuration from config.json
            LoadConfiguration();

            LogMessage("WPF Zoom SDK Demo initialized with C++/CLI wrapper");
        }

        #region Event Handlers

        private void btnInitialize_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                SetButtonsEnabled(false);
                LogMessage("Initializing Zoom SDK...");

                SDKError result = _zoomSDK.Initialize();

                if (result == SDKError.SDKERR_SUCCESS)
                {
                    LogMessage("Zoom SDK initialized successfully");
                    btnInitialize.IsEnabled = false;
                    btnAuthenticate.IsEnabled = true;
                }
                else
                {
                    LogMessage($"Failed to initialize Zoom SDK: {result}");
                    SetButtonsEnabled(true);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Initialization error: {ex.Message}");
                SetButtonsEnabled(true);
            }
        }

        private void btnAuthenticate_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (!_zoomSDK.IsInitialized)
                {
                    LogMessage("Error: SDK not initialized");
                    return;
                }

                SetButtonsEnabled(false);
                LogMessage("Authenticating with Zoom...");

                string jwtToken = txtJwtToken.Text.Trim();
                if (string.IsNullOrEmpty(jwtToken))
                {
                    LogMessage("Error: JWT token is required");
                    SetButtonsEnabled(true);
                    return;
                }

                SDKError result = _zoomSDK.Authenticate(jwtToken);
                if (result == SDKError.SDKERR_SUCCESS)
                {
                    LogMessage("Authentication successful");
                    btnAuthenticate.IsEnabled = false;
                    btnJoinMeeting.IsEnabled = true;
                    btnStartMeeting.IsEnabled = true;
                }
                else
                {
                    LogMessage($"Authentication failed: {result}");
                    SetButtonsEnabled(true);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Authentication error: {ex.Message}");
                SetButtonsEnabled(true);
            }
        }

        private void btnJoinMeeting_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (!_zoomSDK.IsInitialized || !_zoomSDK.IsAuthenticated)
                {
                    LogMessage("Error: SDK not initialized or authenticated");
                    return;
                }

                SetButtonsEnabled(false);
                LogMessage("Joining meeting...");

                if (!ulong.TryParse(txtMeetingNumber.Text.Trim(), out ulong meetingNumber))
                {
                    LogMessage("Error: Invalid meeting number");
                    SetButtonsEnabled(true);
                    return;
                }

                string passcode = txtPasscode.Text.Trim();
                string userName = txtUserName.Text.Trim();

                if (string.IsNullOrEmpty(userName))
                {
                    userName = "WPF User";
                }

                SDKError result = _zoomSDK.JoinMeeting(meetingNumber, passcode, userName);

                if (result == SDKError.SDKERR_SUCCESS)
                {
                    LogMessage("Join meeting request successful");
                    UpdateMeetingControls(true);
                    UpdateMeetingInfo();
                }
                else
                {
                    LogMessage($"Join meeting failed: {result}");
                    SetButtonsEnabled(true);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Join meeting error: {ex.Message}");
                SetButtonsEnabled(true);
            }
        }

        private void btnStartMeeting_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (!_zoomSDK.IsInitialized || !_zoomSDK.IsAuthenticated)
                {
                    LogMessage("Error: SDK not initialized or authenticated");
                    return;
                }

                SetButtonsEnabled(false);
                LogMessage("Starting meeting...");

                if (!ulong.TryParse(txtMeetingNumber.Text.Trim(), out ulong meetingNumber))
                {
                    LogMessage("Error: Invalid meeting number");
                    SetButtonsEnabled(true);
                    return;
                }

                string userName = txtUserName.Text.Trim();
                if (string.IsNullOrEmpty(userName))
                {
                    userName = "WPF Host";
                }

                SDKError result = _zoomSDK.StartMeeting(meetingNumber, userName);

                if (result == SDKError.SDKERR_SUCCESS)
                {
                    LogMessage("Start meeting request successful");
                    UpdateMeetingControls(true);
                    UpdateMeetingInfo();
                }
                else
                {
                    LogMessage($"Start meeting failed: {result}");
                    SetButtonsEnabled(true);
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Start meeting error: {ex.Message}");
                SetButtonsEnabled(true);
            }
        }

        private void btnLeaveMeeting_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                LogMessage("Leaving meeting...");

                SDKError result = _zoomSDK.LeaveMeeting();
                if (result == SDKError.SDKERR_SUCCESS)
                {
                    LogMessage("Left meeting");
                    UpdateMeetingControls(false);
                }
                else
                {
                    LogMessage($"Leave meeting failed: {result}");
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Leave meeting error: {ex.Message}");
            }
        }

        private void btnEndMeeting_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                LogMessage("Ending meeting...");

                SDKError result = _zoomSDK.EndMeeting();
                if (result == SDKError.SDKERR_SUCCESS)
                {
                    LogMessage("Ended meeting");
                    UpdateMeetingControls(false);
                }
                else
                {
                    LogMessage($"End meeting failed: {result}");
                }
            }
            catch (Exception ex)
            {
                LogMessage($"End meeting error: {ex.Message}");
            }
        }

        private void btnMuteAudio_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                bool isMuted = _zoomSDK.IsAudioMuted();
                SDKError result = _zoomSDK.MuteAudio(!isMuted);

                if (result == SDKError.SDKERR_SUCCESS)
                {
                    string action = !isMuted ? "muted" : "unmuted";
                    LogMessage($"Audio {action}");
                    UpdateMuteStatus();
                }
                else
                {
                    LogMessage($"Audio control failed: {result}");
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Audio control error: {ex.Message}");
            }
        }

        private void btnMuteVideo_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                bool isMuted = _zoomSDK.IsVideoMuted();
                SDKError result = _zoomSDK.MuteVideo(!isMuted);

                if (result == SDKError.SDKERR_SUCCESS)
                {
                    string action = !isMuted ? "muted" : "unmuted";
                    LogMessage($"Video {action}");
                    UpdateMuteStatus();
                }
                else
                {
                    LogMessage($"Video control failed: {result}");
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Video control error: {ex.Message}");
            }
        }

        #endregion

        #region Helper Methods

        private void LogMessage(string message)
        {
            Dispatcher.Invoke(() =>
            {
                string timestamp = DateTime.Now.ToString("HH:mm:ss");
                txtLog.AppendText($"[{timestamp}] {message}\r\n");
                txtLog.ScrollToEnd();
            });
        }

        private void SetButtonsEnabled(bool enabled)
        {
            Dispatcher.Invoke(() =>
            {
                btnInitialize.IsEnabled = enabled && !_zoomSDK.IsInitialized;
                btnAuthenticate.IsEnabled = enabled && _zoomSDK.IsInitialized && !_zoomSDK.IsAuthenticated;
                btnJoinMeeting.IsEnabled = enabled && _zoomSDK.IsAuthenticated;
                btnStartMeeting.IsEnabled = enabled && _zoomSDK.IsAuthenticated;
            });
        }

        private void UpdateMeetingControls(bool inMeeting)
        {
            Dispatcher.Invoke(() =>
            {
                btnMuteAudio.IsEnabled = inMeeting;
                btnMuteVideo.IsEnabled = inMeeting;
                btnLeaveMeeting.IsEnabled = inMeeting;
                btnEndMeeting.IsEnabled = inMeeting;

                if (inMeeting)
                {
                    UpdateMuteStatus();
                }
                else
                {
                    chkAudioMuted.IsChecked = false;
                    chkVideoMuted.IsChecked = false;
                    _participants.Clear();
                }
            });
        }

        private void UpdateMuteStatus()
        {
            chkAudioMuted.IsChecked = _zoomSDK.IsAudioMuted();
            chkVideoMuted.IsChecked = _zoomSDK.IsVideoMuted();
        }

        private void UpdateMeetingInfo()
        {
            try
            {
                ulong meetingId = _zoomSDK.GetMeetingID();
                int participantCount = _zoomSDK.GetParticipantCount();
                LogMessage($"Meeting ID: {meetingId}, Participants: {participantCount}");
            }
            catch (Exception ex)
            {
                LogMessage($"Error getting meeting info: {ex.Message}");
            }
        }

        private void LoadConfiguration()
        {
            try
            {
                // Try multiple possible locations for config.json
                string[] possiblePaths = new[]
                {
                    "config.json",  // Current directory
                    Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "config.json"),  // Executable directory
                    Path.Combine(Directory.GetCurrentDirectory(), "config.json"),  // Working directory
                    Path.Combine(Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) ?? "", "config.json")  // Assembly directory
                };

                string configPath = null;
                foreach (string path in possiblePaths)
                {
                    if (File.Exists(path))
                    {
                        configPath = path;
                        break;
                    }
                }

                if (configPath != null)
                {
                    string jsonContent = File.ReadAllText(configPath);
                    var config = JsonSerializer.Deserialize<ConfigData>(jsonContent);

                    if (config != null)
                    {
                        // Use Dispatcher to ensure UI updates happen on UI thread
                        Dispatcher.Invoke(() =>
                        {
                            txtJwtToken.Text = config.SdkJwt ?? "";
                            txtMeetingNumber.Text = config.MeetingNumber ?? "";
                            txtPasscode.Text = config.Passcode ?? "";
                            txtUserName.Text = config.UserName ?? "";
                        });

                        LogMessage($"Configuration loaded from: {Path.GetFileName(configPath)}");
                        LogMessage($"JWT: {(string.IsNullOrEmpty(config.SdkJwt) ? "Not set" : "Loaded")}");
                        LogMessage($"Meeting: {config.MeetingNumber ?? "Not set"}");
                        LogMessage($"Passcode: {(string.IsNullOrEmpty(config.Passcode) ? "Not set" : "Loaded")}");
                        LogMessage($"User: {config.UserName ?? "Not set"}");
                    }
                    else
                    {
                        LogMessage("Warning: Could not parse config.json - invalid JSON format");
                    }
                }
                else
                {
                    LogMessage("Warning: config.json not found in any expected location");
                    LogMessage("Searched locations:");
                    foreach (string path in possiblePaths)
                    {
                        LogMessage($"  - {path}");
                    }
                }
            }
            catch (Exception ex)
            {
                LogMessage($"Error loading configuration: {ex.Message}");
            }
        }

        #endregion

        #region Configuration Classes

        private class ConfigData
        {
            public string? SdkJwt { get; set; }
            public string? MeetingNumber { get; set; }
            public string? Passcode { get; set; }
            public string? UserName { get; set; }
        }

        #endregion

        #region Window Events

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // Ensure configuration is loaded when UI is fully loaded
            LoadConfiguration();
        }

        protected override void OnClosed(EventArgs e)
        {
            // Clean up
            try
            {
                _zoomSDK?.Cleanup();
                _zoomSDK = null;
            }
            catch (Exception ex)
            {
                LogMessage($"Cleanup error: {ex.Message}");
            }
            base.OnClosed(e);
        }

        #endregion
    }
}
