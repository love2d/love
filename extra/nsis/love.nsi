!include "MUI.nsh" # Modern look, plxz

Name "LOVE"
OutFile "love-0.5-0.exe"
Icon "love.ico" # this doesn't seem to work

InstallDir $PROGRAMFILES\LOVE
InstallDirRegKey HKCU "Software\LOVE" ""

# Graphics
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "top.bmp" # optional
!define MUI_WELCOMEFINISHPAGE_BITMAP "left.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "left.bmp"

# Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\license.txt"
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

	# Program
	File ..\..\platform\msvc\Release\love.exe
	
	# Modules
	#File ..\..\platform\msvc\Release\love_chipmunk.dll
	#File ..\..\platform\msvc\Release\love_opengl.dll
	#File ..\..\platform\msvc\Release\love_physfs.dll
	#File ..\..\platform\msvc\Release\love_sdlkeyboard.dll
	#File ..\..\platform\msvc\Release\love_sdlmixer.dll
	#File ..\..\platform\msvc\Release\love_sdlmouse.dll
	#File ..\..\platform\msvc\Release\love_sdltimer.dll
	#File ..\..\platform\msvc\Release\love_system.dll
	
	# DLLs
	File ..\..\platform\msvc\DevIL.dll
	File ..\..\platform\msvc\ILU.dll
	File ..\..\platform\msvc\SDL.dll
	File ..\..\platform\msvc\SDL_mixer.dll
	File ..\..\platform\msvc\Microsoft.VC80.CRT.manifest
	File ..\..\platform\msvc\msvcp80.dll
	File ..\..\platform\msvc\msvcr80.dll
	
	# Delete old DLLS (from 0.2.1 - 0.3.1)
	Delete $INSTDIR\love_chipmunk.dll
	Delete $INSTDIR\love_opengl.dll
	Delete $INSTDIR\love_physfs.dll
	Delete $INSTDIR\love_sdlkeyboard.dll
	Delete $INSTDIR\love_sdlmixer.dll
	Delete $INSTDIR\love_sdlmouse.dll
	Delete $INSTDIR\love_sdltimer.dll
	Delete $INSTDIR\love_system.dll
	Delete $INSTDIR\freetype6.dll
	Delete $INSTDIR\ILUT.dll
	Delete $INSTDIR\libogg-0.dll
	Delete $INSTDIR\libvorbis-0.dll
	Delete $INSTDIR\libvorbisfile-3.dll
	Delete $INSTDIR\lua5.1.dll
	Delete $INSTDIR\physfs.dll
	Delete $INSTDIR\smpeg.dll
	Delete $INSTDIR\zlib1.dll
	
	# Delete old demos.
	Delete $INSTDIR\demos\kkav.love
	Delete $INSTDIR\demos\passingclouds.love
	Delete $INSTDIR\demos\sinescroller.love
	Delete $INSTDIR\demos\no.love
	Delete $INSTDIR\demos\particles.love

	# Icons
	File love.ico
	File game.ico
	# Text
	File ..\..\changes.txt
	File ..\..\license.txt
	File ..\..\readme.txt

	# Uninstaller
	WriteUninstaller $INSTDIR\Uninstall.exe

	# Start Menu
	# Remove old menu items (from 0.2.0)
	Delete "$SMPROGRAMS\LOVE\Demos\Animation Demo.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Chinchilla.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Color Blend Demo.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Hello LOVE.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Mouse and Image Basics.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Movement.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Passing Clouds.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Sound.lnk"
	Delete "$SMPROGRAMS\LOVE\Demos\Text Demo.lnk"
	RMDir "$SMPROGRAMS\LOVE\Demos"
	Delete "$SMPROGRAMS\LOVE\LOVE.lnk"
	Delete "$SMPROGRAMS\LOVE\Uninstall.lnk"
	Delete "$SMPROGRAMS\LOVE\Documentation.url"
	# Add new start menu items
	CreateDirectory "$SMPROGRAMS\LOVE"
	CreateShortCut "$SMPROGRAMS\LOVE\LOVE.lnk" "$INSTDIR\love.exe"
	CreateShortCut "$SMPROGRAMS\LOVE\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	WriteINIStr "$SMPROGRAMS\LOVE\Documentation.url" "InternetShortcut" "URL" "http://love.sourceforge.net/docs/"

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

# Demos
Section "Demos" Demos

	SetOutPath $INSTDIR\demos

	# Files
	File ..\..\demos\lovalanche.love
	File ..\..\demos\no.love
	File ..\..\demos\particles.love

	# Start menu
	CreateShortCut "$SMPROGRAMS\LOVE\Demos.lnk" "$INSTDIR\demos"

SectionEnd

# Descriptions
LangString DESC_MainProg ${LANG_ENGLISH} "Main program."
LangString DESC_Demos ${LANG_ENGLISH} "A series of demos to show what LOVE can do."

# Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${MainProg} $(DESC_MainProg)
	!insertmacro MUI_DESCRIPTION_TEXT ${Demos} $(DESC_Demos)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# Uninstall
Section "Uninstall"

	# Delete Demos
	Delete $INSTDIR\demos\lovalanche.love
	Delete $INSTDIR\demos\no.love
	Delete $INSTDIR\demos\particles.love

	RMDir $INSTDIR\demos
	# Program
	Delete $INSTDIR\love.exe
	# Modules
	# Delete $INSTDIR\love_chipmunk.dll
	# Delete $INSTDIR\love_opengl.dll
	# Delete $INSTDIR\love_physfs.dll
	# Delete $INSTDIR\love_sdlkeyboard.dll
	# Delete $INSTDIR\love_sdlmixer.dll
	# Delete $INSTDIR\love_sdlmouse.dll
	# Delete $INSTDIR\love_sdltimer.dll
	# Delete $INSTDIR\love_system.dll
	# DLLs
	Delete $INSTDIR\DevIL.dll
	Delete $INSTDIR\ILU.dll
	Delete $INSTDIR\SDL.dll
	Delete $INSTDIR\SDL_mixer.dll
	Delete $INSTDIR\Microsoft.VC80.CRT.manifest
	Delete $INSTDIR\msvcp80.dll
	Delete $INSTDIR\msvcr80.dll
	# Icons
	Delete $INSTDIR\love.ico
	Delete $INSTDIR\game.ico
	# Text
	Delete $INSTDIR\changes.txt
	Delete $INSTDIR\license.txt
	Delete $INSTDIR\readme.txt
	RMDir $INSTDIR

	# Start menu
	Delete "$SMPROGRAMS\LOVE\Demos.lnk"
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