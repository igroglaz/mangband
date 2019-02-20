; Inno install packager - http://www.jrsoftware.org/isinfo.php

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{22222C83-6BF9-4372-9226-73768939CB3D}}
AppName=MAngband
#define VERSION "1.5.0a"
AppVersion={#VERSION}
AppPublisher=MAngband Project Team
AppPublisherURL=https://mangband.org
AppSupportURL=
AppUpdatesURL=
DefaultDirName={localappdata}\MAngband1.5
DefaultGroupName=MAngband-Devel
PrivilegesRequired=lowest
OutputDir=.
OutputBaseFilename=mangband-client-setup-v{#VERSION}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\mangclient.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\mangclient-sdl.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\sdl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\mangclient.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "README-client.htm"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\lib\pref\*"; DestDir: "{app}\lib\pref"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\user\*"; DestDir: "{app}\lib\user"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\xtra\*"; DestDir: "{app}\lib\xtra"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\file\*"; DestDir: "{app}\lib\file"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\help\*"; DestDir: "{app}\lib\help"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\lib\xtra\*"; DestDir: "{app}\lib\xtra"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\MAngband"; Filename: "{app}\mangclient.exe"
Name: "{group}\MAngband (One Window)"; Filename: "{app}\mangclient-sdl.exe"
Name: "{group}\ReadMe"; Filename: "{app}\README-client.htm"
Name: "{group}\{cm:UninstallProgram,MAngband}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\MAngband"; Filename: "{app}\mangclient.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\mangclient.exe"; Description: "{cm:LaunchProgram,MAngband}"; Flags: nowait postinstall skipifsilent
