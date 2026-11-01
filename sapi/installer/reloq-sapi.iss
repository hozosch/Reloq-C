#ifndef AppVersion
  #define AppVersion "0.2.205"
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
AllowNoIcons=no
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
en.UserGuide=User guide
de.VoiceSettings=Stimmeinstellungen
de.UserGuide=Anleitung
es.VoiceSettings=Ajustes de voz
es.UserGuide=Guía del usuario
fr.VoiceSettings=Paramètres de voix
fr.UserGuide=Guide d’utilisation
it.VoiceSettings=Impostazioni voce
it.UserGuide=Guida all’uso
pt.VoiceSettings=Configurações de voz
pt.UserGuide=Guia do usuário
fi.VoiceSettings=Ääniasetukset
fi.UserGuide=Käyttöohje
el.VoiceSettings=Ρυθμίσεις φωνής
el.UserGuide=Οδηγίες χρήσης
ja.VoiceSettings=音声設定
ja.UserGuide=利用ガイド
ko.VoiceSettings=음성 설정
ko.UserGuide=사용 안내
zh.VoiceSettings=语音设置
zh.UserGuide=使用指南
zh_tw.VoiceSettings=語音設定
zh_tw.UserGuide=使用指南

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
Name: "{group}\ReLoq C - {cm:UserGuide}"; Filename: "{app}\README.txt"
Name: "{group}\ReLoq C - {cm:VoiceSettings}"; Filename: "{app}\ReLoqSettings.exe"; Check: Is64BitInstallMode
Name: "{group}\ReLoq C - {cm:VoiceSettings}"; Filename: "{app}\x86\ReLoqSettings.exe"; Check: not Is64BitInstallMode
Name: "{group}\{cm:UninstallProgram,ReLoq C SAPI5}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\ReLoqSettings.exe"; Description: "{cm:VoiceSettings}"; Flags: postinstall nowait skipifsilent runasoriginaluser; Check: Is64BitInstallMode
Filename: "{app}\x86\ReLoqSettings.exe"; Description: "{cm:VoiceSettings}"; Flags: postinstall nowait skipifsilent runasoriginaluser; Check: not Is64BitInstallMode

[Code]
procedure RemovePreviousLinks;
var
  Search: TFindRec;
  Shell, Link: Variant;
  FileName, Target, AppDir, BaseName: String;
begin
  { Rebuild owned links when the selected installer language changes. Keep
    unrelated user shortcuts in the group. Reading a link never saves it. }
  if not FindFirst(ExpandConstant('{group}\*.lnk'), Search) then Exit;
  try
    try
      Shell := CreateOleObject('WScript.Shell');
      AppDir := ExpandConstant('{app}');
      repeat
        FileName := ExpandConstant('{group}\') + Search.Name;
        try
          Link := Shell.CreateShortcut(FileName);
          Target := Link.TargetPath;
          BaseName := Lowercase(ExtractFileName(Target));
          if ((CompareText(ExtractFileDir(Target), AppDir) = 0) and
              ((BaseName = 'reloqsettings.exe') or (BaseName = 'readme.txt') or
               ((Pos('unins', BaseName) = 1) and (ExtractFileExt(BaseName) = '.exe')))) or
             (CompareText(Target, AppDir + '\x86\ReLoqSettings.exe') = 0) then
            DeleteFile(FileName);
        except
          Log('Previous shortcut retained: ' + GetExceptionMessage);
        end;
      until not FindNext(Search);
    except
      Log('Previous shortcut cleanup unavailable: ' + GetExceptionMessage);
    end;
  finally
    FindClose(Search);
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then RemovePreviousLinks;
end;
