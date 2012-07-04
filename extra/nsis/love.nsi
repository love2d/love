!include "MUI2.nsh" # Modern look, plxz

Name "LOVE"

OutFile ${LOVEEXE}

InstallDir $PROGRAMFILES\LOVE
InstallDirRegKey HKCU "Software\LOVE" ""

# Graphics
!define MUI_ICON "${LOVEICODIR}\love.ico"
!define MUI_UNICON "${LOVEICODIR}\love.ico"
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${LOVEICODIR}\top.bmp" # optional
!define MUI_WELCOMEFINISHPAGE_BITMAP "${LOVEICODIR}\left.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${LOVEICODIR}\left.bmp"

!define MUI_WELCOMEPAGE_TITLE "LÖVE Setup"
!define MUI_WELCOMEPAGE_TEXT "This will install LÖVE, the unquestionably awesome Lua game framework."

# Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${LOVELICDIR}\license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Languages
!insertmacro MUI_LANGUAGE "English"

# Main Program
Section "LOVE" MainProg

	SectionIn RO
	SetOutPath $INSTDIR
	File "${LOVEBINDIR}\love.exe"
	File "${LOVEBINDIR}\DevIL.dll"
	File "${LOVEBINDIR}\SDL.dll"
	File "${LOVEBINDIR}\OpenAL32.dll"
	File "${LOVEBINDIR}\libmpg123.dll"
	File "${LOVEBINDIR}\gme.dll"
	# File "${LOVEBINDIR}\lua51.dll"
	File "${LOVEICODIR}\love.ico"
	File "${LOVEICODIR}\game.ico"

	# Uninstaller
	WriteUninstaller $INSTDIR\Uninstall.exe

	# Add new start menu items
	CreateDirectory "$SMPROGRAMS\LOVE"
	CreateShortCut "$SMPROGRAMS\LOVE\LOVE.lnk" "$INSTDIR\love.exe"
	CreateShortCut "$SMPROGRAMS\LOVE\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	WriteINIStr "$SMPROGRAMS\LOVE\Documentation.url" "InternetShortcut" "URL" "http://love2d.org/wiki/"

	# Desktop
	CreateShortCut "$DESKTOP\LOVE.lnk" "$INSTDIR\love.exe" ""

	# Registry
	WriteRegStr HKCU "Software\LOVE" "" $INSTDIR
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LOVE" "DisplayName" "LOVE (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LOVE" "UninstallString" "$INSTDIR\Uninstall.exe"
	
	# love file association
	WriteRegStr HKCR ".love" "" "LOVE"
	WriteRegStr HKCR "LOVE" "" "LOVE Game File"
	WriteRegStr HKCR "LOVE\DefaultIcon" "" "$INSTDIR\game.ico"
	WriteRegStr HKCR "LOVE\shell" "" "open"
	WriteRegStr HKCR "LOVE\shell\open" "" "Open in LOVE"
	WriteRegStr HKCR "LOVE\shell\open\command" "" "$INSTDIR\love.exe $\"%1$\""
	# Refresh shell icons
	System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v  (0x08000000, 0, 0, 0)'

SectionEnd

# Descriptions
LangString DESC_MainProg ${LANG_ENGLISH} "Main program."

# Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${MainProg} $(DESC_MainProg)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# Uninstall
Section "Uninstall"
	Delete $INSTDIR\"DevIL.dll"
	Delete $INSTDIR\"SDL.dll"
	Delete $INSTDIR\"love.exe"
	Delete $INSTDIR\"OpenAL32.dll"
	Delete $INSTDIR\"libmpg123.dll"
	Delete $INSTDIR\"gme.dll"
	# Delete $INSTDIR\"lua51.dll"
	Delete $INSTDIR\"game.ico"
	Delete $INSTDIR\"love.ico"
	RMDir $INSTDIR
	
	# Start menu
	Delete "$SMPROGRAMS\LOVE\LOVE.lnk"
	Delete "$SMPROGRAMS\LOVE\Uninstall.lnk"
	Delete "$SMPROGRAMS\LOVE\Documentation.url"
	RMDir "$SMPROGRAMS\LOVE"
	
	# Desktop
	Delete "$DESKTOP\LOVE.lnk"

	# Uninstall keys
	DeleteRegKey /ifempty HKCU "Software\LOVE"
	DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\LOVE"
	
	# love file association
	DeleteRegKey HKCR "LOVE"
	DeleteRegKey HKCR ".love"
	# Refresh shell icons
	System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v  (0x08000000, 0, 0, 0)'

SectionEnd
