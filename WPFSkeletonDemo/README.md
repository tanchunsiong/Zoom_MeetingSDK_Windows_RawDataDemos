# WPF Zoom Meeting SDK Demo

This project provides a WPF (Windows Presentation Foundation) interface for the Zoom Meeting SDK, featuring a C++/CLI wrapper that enables seamless integration between the native Zoom SDK and managed .NET code.

## Project Structure

```
WPFSkeletonDemo/
├── ZoomSDKWrapper/              # C++/CLI wrapper DLL
│   ├── ZoomSDKWrapper.vcxproj   # C++ project file
│   ├── ZoomSDKWrapper.h         # Wrapper header
│   ├── ZoomSDKWrapper.cpp       # Wrapper implementation
│   └── ZoomSDKWrapper.def       # Module definition
├── WPFSkeletonDemo/             # WPF application
│   ├── WPFSkeletonDemo.csproj   # .NET project file
│   ├── MainWindow.xaml          # Main UI layout
│   ├── MainWindow.xaml.cs       # Main window logic
│   ├── App.xaml                 # Application resources
│   ├── App.xaml.cs              # Application logic
│   └── config.json              # Configuration file
├── WPFSkeletonDemo.sln          # Visual Studio solution
├── .gitignore                   # Git ignore rules
└── README.md                    # This file
```

## Features

- **Custom UI Mode**: Full control over the meeting interface
- **C++ to C# Wrapper**: Seamless interop between native Zoom SDK and managed WPF application
- **Real-time Events**: Live updates for meeting status, participants, and audio/video controls
- **Meeting Management**: Join, start, leave, and end meetings
- **Audio/Video Controls**: Mute/unmute audio and video
- **Participant Management**: View and manage meeting participants

## Prerequisites

- **Visual Studio 2022** with:
  - .NET desktop development workload
  - Desktop development with C++ workload
  - Windows SDK 10.0 or later
- **Zoom Meeting SDK** (referenced from parent directory `../SDK/x64/`)
- **Windows 10/11** (x64 architecture only)
- **.NET 6.0** or later

## Quick Start

### 1. Build the Solution

**Important**: Use MSBuild instead of dotnet build for mixed C++/C# projects:

```bash
# Build the entire solution
msbuild WPFSkeletonDemo.sln /p:Configuration=Debug /p:Platform=x64

# Or build individual projects in order:
msbuild ZoomSDKWrapper\ZoomSDKWrapper.vcxproj /p:Configuration=Debug /p:Platform=x64
msbuild WPFSkeletonDemo\WPFSkeletonDemo.csproj /p:Configuration=Debug /p:Platform=x64
```

### 2. Configure Meeting Settings

Edit `WPFSkeletonDemo/config.json`:

```json
{
  "SdkJwt": "your_jwt_token_here",
  "MeetingNumber": "4046552925",
  "Passcode": "your_meeting_passcode",
  "UserName": "Your Display Name"
}
```

### 3. Run the Application

```bash
# From the project root directory
start WPFSkeletonDemo\bin\x64\Debug\net6.0-windows\WPFSkeletonDemo.exe
```

## SDK Integration Details

### Zoom SDK Paths
The project expects the Zoom SDK to be located at:
- **Headers**: `../SDK/x64/h/`
- **Libraries**: `../SDK/x64/lib/`
- **Binaries**: `../SDK/x64/bin/`

### Build Configuration
- **Platform**: x64 only (required for Zoom SDK)
- **Configuration**: Debug/Release
- **Target Framework**: .NET 6.0 Windows
- **C++ Toolset**: v143 (Visual Studio 2022)

## Usage

### 1. Initialize SDK
- Click "Initialize SDK" to set up the Zoom SDK
- The application will show initialization status

### 2. Authenticate
- Click "Authenticate" to authenticate with your JWT token
- Wait for authentication confirmation

### 3. Join or Start Meeting
- **Join Meeting**: Enter meeting number and passcode, then click "Join Meeting"
- **Start Meeting**: Enter meeting number and click "Start Meeting"

### 4. Meeting Controls
- **Audio/Video**: Use mute/unmute buttons to control audio and video
- **Leave Meeting**: Gracefully leave the current meeting
- **End Meeting**: End the meeting (host only)

### 5. Monitor Participants
- View real-time participant list
- See participant status and roles

## Key Components

### ZoomSDKWrapper (C++/CLI DLL)
- **Direct C++/CLI wrapper** around Zoom Meeting SDK
- **Managed interface** (`ZoomSDKManager` class) for C# consumption
- **SDK lifecycle management**: initialization, authentication, cleanup
- **Meeting operations**: join, start, leave, end meetings
- **Audio/Video controls**: mute/unmute functionality
- **Participant management**: get participant lists and info
- **Automatic SDK dependency copying**: post-build events copy required DLLs

### WPFSkeletonDemo (WPF Application)
- **Main UI window** with meeting controls and status display
- **Real-time logging** with timestamps for all operations
- **Configuration loading** from `config.json`
- **Event-driven updates** for meeting status and participants
- **Error handling** with user-friendly messages
- **Participant list display** with user information

## Custom UI Mode

This implementation supports Zoom's custom UI mode, allowing you to:
- Create custom meeting interfaces
- Integrate meeting functionality into existing applications
- Control meeting layout and behavior
- Handle meeting events programmatically

## Error Handling

The application includes comprehensive error handling:
- SDK initialization errors
- Authentication failures
- Meeting connection issues
- Network problems
- Invalid parameters

## Logging

All operations are logged with timestamps in the application window, helping with:
- Debugging connection issues
- Monitoring meeting status
- Tracking participant changes
- Identifying errors

## Differences from Original SkeletonDemo

### Architecture Changes
- **UI Framework**: WPF desktop application instead of console-based interface
- **Wrapper Technology**: C++/CLI mixed-mode DLL instead of pure C++ application
- **Language Integration**: Seamless C# ↔ C++ interop using managed wrapper classes
- **Project Structure**: Two-project solution (C++ wrapper + WPF app) instead of single executable

### Functional Enhancements
- **Real-time UI Updates**: WPF data binding and Dispatcher for thread-safe UI updates
- **Configuration Management**: JSON-based configuration loading with error handling
- **Comprehensive Logging**: Timestamped operation logging with UI display
- **Participant Display**: Live participant list with user information
- **Meeting Status Tracking**: Visual indicators for meeting state and connection status

### Technical Improvements
- **Memory Management**: Proper resource cleanup and exception handling
- **Build Process**: Automated dependency copying via MSBuild post-build events
- **Platform Consistency**: x64-only configuration for Zoom SDK compatibility
- **Error Recovery**: Graceful error handling with user-friendly messages
- **Git Integration**: Comprehensive .gitignore to prevent binary file commits

### UI/UX Features
- **Modern Interface**: Windows 11-style WPF application with proper theming
- **Responsive Controls**: Enabled/disabled states based on SDK status
- **Status Indicators**: Visual feedback for audio/video mute states
- **Real-time Monitoring**: Live updates for participant changes and meeting events

## Git Configuration

The project includes a comprehensive `.gitignore` file that excludes:

### Build Outputs
- `bin/`, `obj/`, `Debug/`, `Release/` directories
- All `.dll`, `.exe` files (including Zoom SDK binaries)

### Source Code Protection
- Header files (`*.h`, `*.hpp`) to protect Zoom SDK headers
- Visual Studio user files (`*.user`, `*.suo`)
- IDE-specific files (`.vs/`, `.vscode/`)

### Why This Matters
- **Prevents accidental commits** of large binary files
- **Protects sensitive SDK headers** from being shared
- **Keeps repository clean** with only source code
- **Reduces repository size** significantly

## Troubleshooting

### Common Issues

1. **"Incorrect configuration mappings" in Visual Studio**
   - Solution: The project uses x64-only configuration. Ensure you're building for x64 platform.
   - Check: Build → Configuration Manager → Active solution platform should be "x64"

2. **SDK Initialization Fails**
   - Verify Zoom SDK is in `../SDK/x64/` directory
   - Ensure Visual Studio has C++ workload installed
   - Check that `msbuild` is used instead of `dotnet build`

3. **"Unable to start debugging"**
   - Solution: Set WPFSkeletonDemo as startup project (right-click → Set as StartUp Project)
   - Ensure Debug|x64 configuration is selected

4. **FileNotFoundException for ZoomSDKWrapper.dll**
   - Build the ZoomSDKWrapper project first
   - Ensure post-build events copied the DLL correctly
   - Check that paths in project references are correct

5. **Authentication Fails**
   - Verify JWT token format and expiration
   - Check token has correct permissions
   - Ensure network connectivity to Zoom services

6. **Meeting Connection Fails**
   - Verify meeting number and passcode
   - Check if meeting is active
   - Ensure stable network connection

### Build Issues

1. **MSBuild vs dotnet build**
   ```bash
   # Use this (correct):
   msbuild WPFSkeletonDemo.sln /p:Configuration=Debug /p:Platform=x64

   # Don't use this (fails with C++ projects):
   dotnet build WPFSkeletonDemo.sln
   ```

2. **Platform Target Mismatch**
   - Always use x64 platform (required by Zoom SDK)
   - Check project properties → Build → Platform target

3. **Missing Dependencies**
   - Ensure Zoom SDK is properly installed in parent directory
   - Verify all required workloads are installed in Visual Studio

### Debug Tips

- **Enable detailed logging**: Check the application log window for detailed error messages
- **Step through initialization**: Use Visual Studio debugger to trace SDK initialization
- **Check Windows Event Viewer**: Look for system-level errors
- **Monitor file copying**: Verify that post-build events successfully copy Zoom SDK DLLs
- **Validate configuration**: Ensure `config.json` has correct format and values

## License

This project follows the same license as the parent Zoom Meeting SDK project.
