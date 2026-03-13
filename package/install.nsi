VIFileVersion "1.0.0.0"
VIProductVersion "1.0.0.0"
VIAddVersionKey "FileVersion" "1.0.0.0"
VIAddVersionKey "LegalCopyright" "3-clause BSD License"
VIAddVersionKey "FileDescription" "Feather HTTPd Installer"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\Japanese.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

Name "Feather HTTPd"
OutFile "install.exe"
InstallDir "C:\Feather"
Icon "../logo/logo.ico"
LicenseData ../LICENSE

!include "x64.nsh"
!include "LogicLib.nsh"
!include "Sections.nsh"

Page license
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section
	CreateDirectory "$INSTDIR"
	SetOutPath "$INSTDIR"
	File /r tmp/C:/Feather/*

	SetOverwrite Off
	File /r tmp/etc
	File /r tmp/www
	SetOverwrite On

	${If} ${RunningX64}
		SetRegView 64
	${Else}
		SetRegView 32
	${EndIf}

	CreateDirectory "$SMPROGRAMS\Feather HTTPd"
	CreateShortcut "$SMPROGRAMS\Feather HTTPd\License.lnk" "$INSTDIR\LICENSE.txt" ""
	CreateShortcut "$SMPROGRAMS\Feather HTTPd\Start Feather HTTPd.lnk" "$INSTDIR\bin\fhttpd.exe" ""
	CreateShortcut "$SMPROGRAMS\Feather HTTPd\Uninstall Feather HTTPd.lnk" "$INSTDIR\uninstall.exe" ""

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Feather HTTPd" "DisplayName" "Feather HTTPd"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Feather HTTPd" "InstallDir" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Feather HTTPd" "UninstallString" '"$INSTDIR\uninstall.exe"'

	WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
	RMDir /r "$INSTDIR"
	RMDir /r "$SMPROGRAMS\Feather HTTPd"

	${If} ${RunningX64}
		SetRegView 64
	${Else}
		SetRegView 32
	${EndIf}
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Feather HTTPd"
SectionEnd
