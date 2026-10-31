#ifndef AppVersion
  #define AppVersion "0.2.204"
#endif
#ifndef Payload
  #define Payload "payload"
#endif
#ifndef Output
  #define Output "..\..\dist"
#endif
[Setup]
AppId={{D3F8BA6E-1C62-4B0D-8A33-3840B77376AC}
AppName=ReLoq C SAPI5
AppVersion={#AppVersion}
AppPublisher=ReLoq C contributors
AppPublisherURL=https://github.com/hozosch/Reloq-C
AppSupportURL=https://github.com/hozosch/Reloq-C/issues
DefaultDirName={autopf}\ReLoq C\SAPI5
DefaultGroupName=ReLoq C SAPI5
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64compatible
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage
UsePreviousLanguage=yes
OutputDir={#Output}
OutputBaseFilename=reloq-c-sapi5-{#AppVersion}-setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
UninstallDisplayIcon={app}\unins000.exe
SetupLogging=yes

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"
Name: "es"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "fr"; MessagesFile: "compiler:Languages\French.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "pt"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "fi"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "el"; MessagesFile: "languages\Greek.isl"
Name: "ja"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "ko"; MessagesFile: "compiler:Languages\Korean.isl"
Name: "zh"; MessagesFile: "languages\ChineseSimplified.isl"
Name: "zh_tw"; MessagesFile: "languages\ChineseTraditional.isl"

[CustomMessages]
en.VoiceSettings=Voice settings
de.VoiceSettings=Stimmeinstellungen
es.VoiceSettings=Ajustes de voz
fr.VoiceSettings=Paramètres de voix
it.VoiceSettings=Impostazioni voce
pt.VoiceSettings=Configurações de voz
fi.VoiceSettings=Ääniasetukset
el.VoiceSettings=Ρυθμίσεις φωνής
ja.VoiceSettings=音声設定
ko.VoiceSettings=음성 설정
zh.VoiceSettings=语音设置
zh_tw.VoiceSettings=語音設定

[Files]
; Both COM registry views use the same settings and dictionary folder.
Source: "{#Payload}\x64\eci.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#Payload}\x64\ReLoqSapi.dll"; DestDir: "{app}"; Flags: ignoreversion regserver 64bit; Check: Is64BitInstallMode
Source: "{#Payload}\x64\ReLoqSettings.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#Payload}\x64\Test-SAPI.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "{#Payload}\x86\eci.dll"; DestDir: "{app}\x86"; Flags: ignoreversion
Source: "{#Payload}\x86\ReLoqSapi.dll"; DestDir: "{app}\x86"; Flags: ignoreversion regserver 32bit
Source: "{#Payload}\x86\ReLoqSettings.exe"; DestDir: "{app}\x86"; Flags: ignoreversion
Source: "{#Payload}\x86\Test-SAPI.exe"; DestDir: "{app}\x86"; Flags: ignoreversion
; Keep existing user edits, including a prior manual x86 installation.
Source: "{commonpf32}\ReLoq C\SAPI5\dictionaries\*.dic"; DestDir: "{app}\dictionaries"; Flags: external skipifsourcedoesntexist onlyifdoesntexist uninsneveruninstall; Check: Is64BitInstallMode
Source: "{#Payload}\common\dictionaries\*"; DestDir: "{app}\dictionaries"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "{#Payload}\common\licenses\*"; DestDir: "{app}\licenses"; Flags: ignoreversion
Source: "{#Payload}\common\README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#Payload}\common\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "Inno-Setup-LICENSE.txt"; DestDir: "{app}\licenses"; Flags: ignoreversion

[InstallDelete]
Type: files; Name: "{app}\Install-SAPI.exe"

[Icons]
Name: "{group}\{cm:VoiceSettings}"; Filename: "{app}\ReLoqSettings.exe"; Check: Is64BitInstallMode
Name: "{group}\{cm:VoiceSettings}"; Filename: "{app}\x86\ReLoqSettings.exe"; Check: not Is64BitInstallMode
Name: "{group}\{cm:UninstallProgram,ReLoq C SAPI5}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\ReLoqSettings.exe"; Description: "{cm:VoiceSettings}"; Flags: postinstall nowait skipifsilent runasoriginaluser; Check: Is64BitInstallMode
Filename: "{app}\x86\ReLoqSettings.exe"; Description: "{cm:VoiceSettings}"; Flags: postinstall nowait skipifsilent runasoriginaluser; Check: not Is64BitInstallMode
