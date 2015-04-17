; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！

#define MyAppName "ProxyClient"
#define MyAppVersion "2.0.1.10"
#define MyFileVersion MyAppVersion
#define MyAppPublisher ""
#define MyAppExeName "ProxyClient.exe"
#define MyAppGroupName "综合视频管理平台\ProxyClient"

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (生成新的GUID，点击 工具|在IDE中生成GUID。)
AppId={{AC8A81F1-D887-41C5-A4FC-EF7B3D81D21B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
VersionInfoVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppGroupName}
AllowNoIcons=yes
OutputDir=./
OutputBaseFilename=ProxySetup(Neutral)
Compression=lzma
SolidCompression=yes
                

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}";

[Files]
Source: "output\ISTask.dll"; DestDir: "{sys}"; Flags: ignoreversion uninsnosharedfileprompt sharedfile; Attribs: hidden
Source: "output\ProxyClient.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "output\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "output\config.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "output\images\*"; DestDir: "{app}\images"; Flags: ignoreversion

; 注意: 不要在任何共享系统文件上使用“Flags: ignoreversion”

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
procedure RedesignWizardForm;
begin
  with WizardForm.SelectProgramGroupPage do
  begin
    ParentShowHint := False;
  end;
end;

procedure InitializeWizard();
begin
  RedesignWizardForm;
  RedesignWizardForm;
  WizardForm.BorderIcons := [biHelp, biSystemMenu]; //与BorderStyle合用，会导致许可协议等页面的RTF文本失效
end;

var
HasRun:HWND;
KeynotExist:boolean;
ResultCode: Integer;
uicmd: String;
function InitializeSetup():Boolean;
begin
begin
  HasRun := 1;
  while HasRun<>0 do
  begin
  KeynotExist:= true;
  if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{AC8A81F1-D887-41C5-A4FC-EF7B3D81D21B}_is1', 'UninstallString', uicmd) then
  begin
    if MsgBox('安装程序检测到该程序已经安装。' #13#13 '请先删除你的应用程序然后单击"是"继续安装, 或按"否"退出！', mbConfirmation, MB_YESNO) = idNO then
    begin
      HasRun := 0;
      KeynotExist := false;
    end
    else
    begin
      KeynotExist:= false;
      Exec(RemoveQuotes(uicmd), '', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);   // 是
    end;
  end
  else
  begin
    HasRun := 0;
  end;
  Result:= KeynotExist
  end;
end;

end;

//卸载初始化
function RunTaskU(FileName: string; bFullpath: Boolean): Boolean;
  external 'RunTask@{sys}\ISTask.dll stdcall delayload uninstallonly';
function KillTaskU(ExeFileName: string): Integer;
  external 'KillTask@{sys}\ISTask.dll stdcall delayload uninstallonly';
  
function InitializeUninstall(): Boolean;
var strAppName: String;
begin
 strAppName := ExpandConstant('{#MyAppExeName}');
if RunTaskU(strAppName, false) then
begin
    MsgBox('卸载程序检测到你的应用程序正在运行。' #13#13 '请先退出你的应用程序，然后再进行卸载！', mbError, MB_OK);
    Result := false;
    UnloadDll(ExpandConstant('{sys}\ISTask.dll'));
end
else
    Result := true;
end;

procedure CurUninstallStepChanged (CurUninstallStep: TUninstallStep );
var ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then begin

  end;
  if CurUninstallStep = usPostUninstall then begin
      DelTree(ExpandConstant('{app}'),True, True, True);
  end;
end;