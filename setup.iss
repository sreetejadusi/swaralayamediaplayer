#define MyAppName "Swaralaya Media Player"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Swaralaya"
#define MyAppExeName "SwaralayaMediaPlayer.exe"
#ifndef MyBuildDir
  #define MyBuildDir "build"
#endif

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{9B7E34F1-6D58-4C82-A3D9-5E91B2F64C7A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=.\Output
OutputBaseFilename=SwaralayaMediaPlayer_Setup_v1.0.0
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; NOTE: Make sure to run `windeployqt` on your executable before compiling this script!
; It will gather all necessary Qt DLLs and plugins into your build directory.
Source: "{#MyBuildDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyBuildDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt plugin directories commonly deployed by windeployqt
Source: "{#MyBuildDir}\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#MyBuildDir}\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#MyBuildDir}\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#MyBuildDir}\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#MyBuildDir}\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "{#MyBuildDir}\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
