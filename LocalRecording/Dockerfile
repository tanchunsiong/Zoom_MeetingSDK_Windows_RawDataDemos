## Use the appropriate Windows base image
#FROM mcr.microsoft.com/windows:20H2
#
## Enable non-interactive installs
#ARG DEBIAN_FRONTEND=noninteractive

ARG REPO=mcr.microsoft.com/dotnet/framework/runtime
FROM $REPO:4.8-windowsservercore-ltsc2019



# Install Chocolatey package manager
RUN powershell -Command \
    Set-ExecutionPolicy Bypass -Scope Process -Force; \
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; \
    iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install dependencies
RUN choco install -y git 
RUN choco install -y cmake 
RUN choco install -y mingw
RUN choco install -y make 
RUN choco install -y wget 
RUN choco install -y unzip 

RUN choco install -y visualstudio2022buildtools --install-arguments \
"--add Microsoft.VisualStudio.Workload.NativeDesktop \
--add Microsoft.VisualStudio.Workload.ManagedDesktopBuildTools \
--add Microsoft.VisualStudio.Component.TestTools.BuildTools \
--add Microsoft.Component.MSBuild \
--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended \
--add Microsoft.VisualStudio.Component.VC.CMake.Project \
--add Microsoft.VisualStudio.Component.VC.ATLMFC   \
--add Microsoft.VisualStudio.Component.VC.ATL \
--add Microsoft.VisualStudio.Component.VC.MFC \
--add Microsoft.VisualStudio.Component.CMake \
--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
--add Microsoft.VisualStudio.Component.Windows10SDK.19041 \
--add Microsoft.VisualStudio.Workload.VCTools \
--add Microsoft.VisualStudio.Component.VC.v143.x86.x64 \
--add Microsoft.VisualStudio.Component.VC.140"

WORKDIR /temp

ADD https://aka.ms/vs/17/release/vs_buildtools.exe vs_buildtools.exe
SHELL ["cmd", "/S", "/C"]

RUN (start /w C:\temp\vs_buildtools.exe --quiet --wait --norestart --nocache modify \
    --installPath "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools" \
    --add    Microsoft.VisualStudio.Workload.ManagedDesktopBuildTools \
    --add    Microsoft.VisualStudio.Workload.VCTools --includeRecommended \
    --add    Microsoft.Component.MSBuild \
    --remove Microsoft.VisualStudio.Component.Windows10SDK.10240 \
    --remove Microsoft.VisualStudio.Component.Windows10SDK.10586 \
    --remove Microsoft.VisualStudio.Component.Windows10SDK.14393 \
    --remove Microsoft.VisualStudio.Component.Windows81SDK \
    || IF "%ERRORLEVEL%"=="3010" EXIT 0)

ENTRYPOINT ["C:\\Program Files (x86)\\Microsoft Visual Studio\\2022\\BuildTools\\Common7\\Tools\\VsDevCmd.bat", "&&", "powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]

WORKDIR /vcpkg
# Clone vcpkg repository
RUN git clone https://github.com/Microsoft/vcpkg.git c:\vcpkg

# Bootstrap vcpkg
RUN .\bootstrap-vcpkg.bat

# Integrate vcpkg
RUN .\vcpkg integrate install --vcpkg-root c:\vcpkg

# Install curl
RUN .\vcpkg install curl

# Install jsoncpp
RUN .\vcpkg install jsoncpp

#
#RUN choco install -y visualstudio2022-workload-manageddesktopbuildtools
#RUN choco install -y vcredist-all
#RUN choco install -y visualcpp-build-tools
#RUN choco install -y vcredist140

RUN choco install -y awscli

 # Set the PATH environment variable
ENV PATH="${PATH};C:\Windows\System32\WindowsPowerShell\v1.0"
ENV PATH="${PATH};C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin"
ENV PATH="${PATH};C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0"
ENV PATH="${PATH};C:\Program Files\Amazon\AWSCLIV2"

# Set the working directory
WORKDIR /app

# Copy everything from the specified directory
COPY / /app/

# Build the project using MSBuild
RUN msbuild /t:Build /p:Configuration=Debug /p:Platform=x64

# Set the working directory
WORKDIR C:/app/x64/Debug

# Set the entry point to run the command prompt
CMD ["powershell.exe", "-Command", "C:/app/x64/Debug/LocalRecording.exe"]

#CMD ["powershell.exe"]
#docker run -it local-recording-image --volume c:\demo2:c:\users\ContainerAdministrator\Documents\zoom