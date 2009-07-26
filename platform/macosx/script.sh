#! /bin/sh
#Use install_name_tool, make love to search libs in its own framework folder instead
#*very* hackish.
#IL
install_name_tool -change /usr/local/lib/libIL.dylib @executable_path/../Frameworks/IL.framework/Versions/A/IL build/Release/love.app/Contents/MacOS/love

install_name_tool -change /usr/local/lib/libILU.dylib @executable_path/../Frameworks/IL.framework/Versions/A/IL build/Release/love.app/Contents/MacOS/love

install_name_tool -change /usr/local/lib/libILUT.dylib @executable_path/../Frameworks/IL.framework/Versions/A/IL build/Release/love.app/Contents/MacOS/love

install_name_tool -change /usr/local/lib/libIL.1.dylib @executable_path/../Frameworks/IL.framework/Versions/A/IL build/Release/love.app/Contents/MacOS/love

install_name_tool -change /usr/local/lib/libILU.1.dylib @executable_path/../Frameworks/IL.framework/Versions/A/IL build/Release/love.app/Contents/MacOS/love

install_name_tool -change /usr/local/lib/libILUT.1.dylib @executable_path/../Frameworks/IL.framework/Versions/A/IL build/Release/love.app/Contents/MacOS/love

# FreeType
install_name_tool -change /Library/Frameworks/FreeType.framework/Versions/2.3/FreeType @executable_path/../Frameworks/FreeType.framework/Versions/2.3/FreeType build/Release/love.app/Contents/MacOS/love

# SDL
install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL build/Release/love.app/Contents/MacOS/love

install_name_tool -change /Library/Frameworks/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL build/Release/love.app/Contents/Frameworks/SDL_mixer.framework/SDL_mixer

# SDL_mixer
#install_name_tool -change /Library/Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer @executable_path/../#Frameworks/SDL_mixer.framework/Versions/A/SDL_mixer build/Release/love.app/Contents/MacOS/love

exit 0