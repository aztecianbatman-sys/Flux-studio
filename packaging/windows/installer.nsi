!include "MUI2.nsh"

Unicode True
RequestExecutionLevel admin

!define APP_NAME "Flux Studio"
!define APP_EXE "Flux Studio.exe"
!define APP_VERSION "0.8.0"
!define PUBLISHER "Flux"
!define INSTALL_DIR "$PROGRAMFILES64\Flux Studio"
!define PROJECT_ROOT "..\.."

Name "${APP_NAME}"
OutFile "${PROJECT_ROOT}\Flux-Studio-${APP_VERSION}-Windows-x64-Setup.exe"
InstallDir "${INSTALL_DIR}"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "InstallLocation"

VIProductVersion "${APP_VERSION}.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "CompanyName" "${PUBLISHER}"
VIAddVersionKey "FileDescription" "Flux Studio professional 2D drawing, animation and compositing workstation"
VIAddVersionKey "FileVersion" "${APP_VERSION}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_WELCOMEPAGE_TITLE "Welcome to Flux Studio"
!define MUI_WELCOMEPAGE_TEXT "Install Flux Studio ${APP_VERSION}, a native professional drawing, animation and compositing workstation."
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch Flux Studio"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Flux Studio" SEC_MAIN
  SectionIn RO
  SetOutPath "$INSTDIR"
  File /r "${PROJECT_ROOT}\dist\*"

  WriteUninstaller "$INSTDIR\Uninstall Flux Studio.exe"
  CreateDirectory "$SMPROGRAMS\Flux Studio"
  CreateShortcut "$SMPROGRAMS\Flux Studio\Flux Studio.lnk" "$INSTDIR\${APP_EXE}"
  CreateShortcut "$SMPROGRAMS\Flux Studio\Uninstall Flux Studio.lnk" "$INSTDIR\Uninstall Flux Studio.exe"
  CreateShortcut "$DESKTOP\Flux Studio.lnk" "$INSTDIR\${APP_EXE}"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "DisplayVersion" "${APP_VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "Publisher" "${PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "UninstallString" '"$INSTDIR\Uninstall Flux Studio.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio" "NoRepair" 1
SectionEnd

Section "Uninstall"
  Delete "$DESKTOP\Flux Studio.lnk"
  Delete "$SMPROGRAMS\Flux Studio\Flux Studio.lnk"
  Delete "$SMPROGRAMS\Flux Studio\Uninstall Flux Studio.lnk"
  RMDir "$SMPROGRAMS\Flux Studio"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FluxStudio"
SectionEnd
