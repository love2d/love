#
# Copyright (c) 2006-2023 LOVE Development Team
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	# Protip: run cmake like this: cmake -G "<generator>" -H. -Bbuild
	message(FATAL_ERROR "Prevented in-tree build.")
endif()

cmake_minimum_required(VERSION 3.1)

project(love)

set(LOVE_EXE_NAME love)
set(LOVE_LIB_NAME liblove)

set(CMAKE_MODULE_PATH "${love_SOURCE_DIR}/extra/cmake" ${CMAKE_MODULE_PATH})
# Needed for shared libs on Linux. (-fPIC).
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set (CMAKE_CXX_STANDARD 11)

if(APPLE)
	message(WARNING "CMake is not an officially supported build system for love on Apple platforms.")
	message(WARNING "Use the prebuilt .app or the xcode project in platform/xcode/ instead.")
endif()

if(MSVC OR MINGW)
	set(LOVE_CONSOLE_EXE_NAME lovec)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(LOVE_X64 TRUE)
	set(LOVE_TARGET_PLATFORM x64)
else()
	set(LOVE_X86 TRUE)
	set(LOVE_TARGET_PLATFORM x86)
endif()

if(APPLE)
	option(LOVE_JIT "Use LuaJIT" FALSE)
else()
	option(LOVE_JIT "Use LuaJIT" TRUE)
endif()
option(LOVE_MPG123 "Use mpg123" TRUE)

if(LOVE_JIT)
	if(APPLE)
		message(WARNING "JIT not supported yet on Mac.")
	endif()
	message(STATUS "LuaJIT: Enabled")
else()
	message(STATUS "LuaJIT: Disabled")
endif()

if(NOT LOVE_MPG123)
	add_definitions(-DLOVE_NOMPG123)
endif()

message(STATUS "Target platform: ${LOVE_TARGET_PLATFORM}")

if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

if(POLICY CMP0063)
	cmake_policy(SET CMP0063 NEW)
endif()

find_package(OpenGL)

if(MEGA)
	# LOVE_MSVC_DLLS contains runtime DLLs that should be bundled with the love
	# binary (in e.g. the installer). Example: msvcp140.dll.
	set(LOVE_MSVC_DLLS ${MEGA_MSVC_DLLS})

	# LOVE_INCLUDE_DIRS contains the search directories for #include. It's mostly
	# not needed for MEGA builds, since almost all the libraries (except LuaJIT)
	# are CMake targets, causing include paths to be added automatically.
	set(LOVE_INCLUDE_DIRS)

	if(APPLE)
		# Some files do #include <SDL2/SDL.h>, but building with megasource
		# requires #include <SDL.h>.
		add_definitions(-DLOVE_MACOSX_SDL_DIRECT_INCLUDE)
	endif ()

	# SDL2 links with some DirectX libraries, and we apparently also
	# pull those libraries in for linkage because we link with SDL2.
	set(LOVE_LINK_DIRS ${SDL_LINK_DIR})

	set(LOVE_LINK_LIBRARIES
		${OPENGL_gl_LIBRARY}
		${MEGA_FREETYPE}
		${MEGA_LIBOGG}
		${MEGA_LIBVORBISFILE}
		${MEGA_LIBVORBIS}
		${MEGA_LIBTHEORA}
		${MEGA_MODPLUG}
		${MEGA_OPENAL}
		${MEGA_SDL2MAIN}
		${MEGA_SDL2}
		${MEGA_ZLIB}
	)

	# These DLLs are moved next to the love binary in a post-build step to
	# love runnable from inside Visual Studio.
	#
	# LOVE_MOVE_DLLS can contain CMake targets, in which case the target's
	# output is assumed to be a DLL, or it can contain paths to actual files.
	# We detect whether or not each item is a target, and take the appropriate
	# action.
	set(LOVE_MOVE_DLLS
		${MEGA_SDL2}
		${MEGA_OPENAL}
	)

	if(LOVE_MPG123)
		set(LOVE_LINK_LIBRARIES
			${LOVE_LINK_LIBRARIES}
			${MEGA_MPEG123}
		)
		set(LOVE_MOVE_DLLS
			${LOVE_MOVE_DLLS}
			${MEGA_MPEG123}
		)
	endif()

	if(LOVE_JIT)
		set(LOVE_LUA_LIBRARY ${MEGA_LUAJIT_LIB})
		# LOVE_EXTRA_DLLS are non-runtime DLLs which should be bundled with the
		# love binary in installers, etc. It's only needed for external
		# (non-CMake) targets, i.e. LuaJIT.
		set(LOVE_EXTRA_DLLS ${MEGA_LUAJIT_DLL})
		set(LOVE_EXTRA_DEPENDECIES luajit)

		set(LOVE_INCLUDE_DIRS
			${LOVE_INCLUDE_DIRS}
			${MEGA_LUAJIT_INCLUDE}
		)
		set(LOVE_LINK_LIBRARIES
			${LOVE_LINK_LIBRARIES}
			${LOVE_LUA_LIBRARY}
		)
		set(LOVE_MOVE_DLLS
			${LOVE_MOVE_DLLS}
			${MEGA_LUAJIT_DLL}
		)
	else()
		set(LOVE_LUA_LIBRARY ${MEGA_LUA51})

		set(LOVE_LINK_LIBRARIES
			${LOVE_LINK_LIBRARIES}
			${LOVE_LUA_LIBRARY}
		)
		set(LOVE_MOVE_DLLS
			${LOVE_MOVE_DLLS}
			${LOVE_LUA_LIBRARY}
		)
		# MEGA_LUA51 is a CMake target, so includes are handled
		# automatically.
	endif()
else()
	if(MSVC)
		message(FATAL_ERROR "
It is currently only possible to build with megasource or (experimental) mingw on Windows.
Please see https://github.com/love2d/megasource
")
	endif()

	find_package(Freetype REQUIRED)
	find_package(ModPlug REQUIRED)
	find_package(OpenAL REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
	find_package(Theora REQUIRED)
	find_package(Vorbis REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(Ogg REQUIRED)

	# required for enet
	add_definitions(-D HAS_SOCKLEN_T)

	set(LOVE_INCLUDE_DIRS
		${SDL2_INCLUDE_DIRS}
		${FREETYPE_INCLUDE_DIRS}
		${VORBIS_INCLUDE_DIR}
		${OPENAL_INCLUDE_DIR}
		${ZLIB_INCLUDE_DIRS}
		${MODPLUG_INCLUDE_DIR}
		${OGG_INCLUDE_DIR}
		${THEORA_INCLUDE_DIR}
	)

	set(LOVE_LINK_LIBRARIES
		${OPENGL_gl_LIBRARY}
		${SDL2_LIBRARIES}
		${FREETYPE_LIBRARY}
		${OPENAL_LIBRARY}
		${MODPLUG_LIBRARY}
		${THEORA_LIBRARY}
		${THEORADEC_LIBRARY}
		${VORBISFILE_LIBRARY}
		${LOVE_LUA_LIBRARY}
		${OGG_LIBRARY}
		${ZLIB_LIBRARY}
	)

	if(LOVE_MPG123)
		find_package(MPG123 REQUIRED)
		set(LOVE_LINK_LIBRARIES
			${LOVE_LINK_LIBRARIES}
			${MPG123_LIBRARY}
		)
		set(LOVE_INCLUDE_DIRS
			${LOVE_INCLUDE_DIRS}
			${MPG123_INCLUDE_DIR}
		)
	endif()

	if(LOVE_JIT)
		find_package(LuaJIT REQUIRED)
		set(LOVE_LUA_LIBRARY ${LUAJIT_LIBRARY})
		set(LOVE_LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIR})
	else()
		find_package(Lua51 REQUIRED)
		set(LOVE_LUA_LIBRARY ${LUA_LIBRARY})
		set(LOVE_LUA_INCLUDE_DIR ${LUA_INCLUDE_DIR})
	endif()

	set(LOVE_INCLUDE_DIRS
		${LOVE_INCLUDE_DIRS}
		${LOVE_LUA_INCLUDE_DIR}
	)
	set(LOVE_LINK_LIBRARIES
		${LOVE_LINK_LIBRARIES}
		${LOVE_LUA_LIBRARY}
	)

endif()

###
### No Megasource-specific stuff beyond this point!
###

if(MSVC)
	set(DISABLE_WARNING_FLAG -W0)
else()
	set(DISABLE_WARNING_FLAG -w)
endif()

function(love_disable_warnings ARG_TARGET)
	get_target_property(OLD_FLAGS ${ARG_TARGET} COMPILE_FLAGS)
	set(NEW_FLAGS ${DISABLE_WARNING_FLAG})
	if(OLD_FLAGS)
		set(NEW_FLAGS "${OLD_FLAGS} ${NEW_FLAGS}")
	endif()
	set_target_properties(${ARG_TARGET} PROPERTIES COMPILE_FLAGS ${NEW_FLAGS})
endfunction()

#
# common
#

set(LOVE_SRC_COMMON
	src/common/b64.cpp
	src/common/b64.h
	src/common/Color.h
	src/common/config.h
	src/common/Data.cpp
	src/common/Data.h
	src/common/delay.cpp
	src/common/delay.h
	src/common/deprecation.cpp
	src/common/deprecation.h
	src/common/EnumMap.h
	src/common/Exception.cpp
	src/common/Exception.h
	src/common/floattypes.cpp
	src/common/floattypes.h
	src/common/int.h
	src/common/math.h
	src/common/Matrix.cpp
	src/common/Matrix.h
	src/common/memory.cpp
	src/common/memory.h
	src/common/Module.cpp
	src/common/Module.h
	src/common/Object.cpp
	src/common/Object.h
	src/common/Optional.h
	src/common/pixelformat.cpp
	src/common/pixelformat.h
	src/common/Reference.cpp
	src/common/Reference.h
	src/common/runtime.cpp
	src/common/runtime.h
	src/common/Stream.cpp
	src/common/Stream.h
	src/common/StringMap.cpp
	src/common/StringMap.h
	src/common/types.cpp
	src/common/types.h
	src/common/utf8.cpp
	src/common/utf8.h
	src/common/Variant.cpp
	src/common/Variant.h
	#src/common/Vector.cpp # Vector.cpp is empty.
	src/common/Vector.h
	src/common/version.h
)

if (APPLE)
	set(LOVE_SRC_COMMON ${LOVE_SRC_COMMON}
		src/common/macosx.mm
	)
	set(LOVE_LINK_LIBRARIES ${LOVE_LINK_LIBRARIES} objc)
	set(LOVE_LINK_LIBRARIES ${LOVE_LINK_LIBRARIES} "-framework CoreFoundation")
	set(LOVE_LINK_LIBRARIES ${LOVE_LINK_LIBRARIES} "-framework AppKit")
endif()

source_group("common" FILES ${LOVE_SRC_COMMON})

#
# love.audio
#

set(LOVE_SRC_MODULE_AUDIO_ROOT
	src/modules/audio/Audio.cpp
	src/modules/audio/Audio.h
	src/modules/audio/Source.cpp
	src/modules/audio/Source.h
	src/modules/audio/RecordingDevice.cpp
	src/modules/audio/RecordingDevice.h
	src/modules/audio/Filter.cpp
	src/modules/audio/Filter.h
	src/modules/audio/Effect.cpp
	src/modules/audio/Effect.h
	src/modules/audio/wrap_Audio.cpp
	src/modules/audio/wrap_Audio.h
	src/modules/audio/wrap_Source.cpp
	src/modules/audio/wrap_Source.h
	src/modules/audio/wrap_RecordingDevice.cpp
	src/modules/audio/wrap_RecordingDevice.h
)

set(LOVE_SRC_MODULE_AUDIO_NULL
	src/modules/audio/null/Audio.cpp
	src/modules/audio/null/Audio.h
	src/modules/audio/null/Source.cpp
	src/modules/audio/null/Source.h
	src/modules/audio/null/RecordingDevice.cpp
	src/modules/audio/null/RecordingDevice.h
)

set(LOVE_SRC_MODULE_AUDIO_OPENAL
	src/modules/audio/openal/Audio.cpp
	src/modules/audio/openal/Audio.h
	src/modules/audio/openal/Pool.cpp
	src/modules/audio/openal/Pool.h
	src/modules/audio/openal/Source.cpp
	src/modules/audio/openal/Source.h
	src/modules/audio/openal/RecordingDevice.cpp
	src/modules/audio/openal/RecordingDevice.h
	src/modules/audio/openal/Filter.cpp
	src/modules/audio/openal/Filter.h
	src/modules/audio/openal/Effect.cpp
	src/modules/audio/openal/Effect.h
)

set(LOVE_SRC_MODULE_AUDIO
	${LOVE_SRC_MODULE_AUDIO_ROOT}
	${LOVE_SRC_MODULE_AUDIO_NULL}
	${LOVE_SRC_MODULE_AUDIO_OPENAL}
)

source_group("modules\\audio" FILES ${LOVE_SRC_MODULE_AUDIO_ROOT})
source_group("modules\\audio\\null" FILES ${LOVE_SRC_MODULE_AUDIO_NULL})
source_group("modules\\audio\\openal" FILES ${LOVE_SRC_MODULE_AUDIO_OPENAL})

#
# love.data
#

set(LOVE_SRC_MODULE_DATA
	src/modules/data/ByteData.cpp
	src/modules/data/ByteData.h
	src/modules/data/CompressedData.cpp
	src/modules/data/CompressedData.h
	src/modules/data/Compressor.cpp
	src/modules/data/Compressor.h
	src/modules/data/DataModule.cpp
	src/modules/data/DataModule.h
	src/modules/data/DataView.cpp
	src/modules/data/DataView.h
	src/modules/data/HashFunction.cpp
	src/modules/data/HashFunction.h
	src/modules/data/wrap_ByteData.cpp
	src/modules/data/wrap_ByteData.h
	src/modules/data/wrap_CompressedData.cpp
	src/modules/data/wrap_CompressedData.h
	src/modules/data/wrap_Data.cpp
	src/modules/data/wrap_Data.h
	src/modules/data/wrap_DataModule.cpp
	src/modules/data/wrap_DataModule.h
	src/modules/data/wrap_DataView.cpp
	src/modules/data/wrap_DataView.h
)

source_group("modules\\data" FILES ${LOVE_SRC_MODULE_DATA})

#
# love.event
#

set(LOVE_SRC_MODULE_EVENT_ROOT
	src/modules/event/Event.cpp
	src/modules/event/Event.h
	src/modules/event/wrap_Event.cpp
	src/modules/event/wrap_Event.h
)

set(LOVE_SRC_MODULE_EVENT_SDL
	src/modules/event/sdl/Event.cpp
	src/modules/event/sdl/Event.h
)

set(LOVE_SRC_MODULE_EVENT
	${LOVE_SRC_MODULE_EVENT_ROOT}
	${LOVE_SRC_MODULE_EVENT_SDL}
)

source_group("modules\\event" FILES ${LOVE_SRC_MODULE_EVENT_ROOT})
source_group("modules\\event\\sdl" FILES ${LOVE_SRC_MODULE_EVENT_SDL})

#
# love.filesystem
#

set(LOVE_SRC_MODULE_FILESYSTEM_ROOT
	src/modules/filesystem/DroppedFile.cpp
	src/modules/filesystem/DroppedFile.h
	src/modules/filesystem/File.cpp
	src/modules/filesystem/File.h
	src/modules/filesystem/FileData.cpp
	src/modules/filesystem/FileData.h
	src/modules/filesystem/Filesystem.cpp
	src/modules/filesystem/Filesystem.h
	src/modules/filesystem/wrap_DroppedFile.cpp
	src/modules/filesystem/wrap_DroppedFile.h
	src/modules/filesystem/wrap_File.cpp
	src/modules/filesystem/wrap_File.h
	src/modules/filesystem/wrap_FileData.cpp
	src/modules/filesystem/wrap_FileData.h
	src/modules/filesystem/wrap_Filesystem.cpp
	src/modules/filesystem/wrap_Filesystem.h
)

set(LOVE_SRC_MODULE_FILESYSTEM_PHYSFS
	src/modules/filesystem/physfs/File.cpp
	src/modules/filesystem/physfs/File.h
	src/modules/filesystem/physfs/Filesystem.cpp
	src/modules/filesystem/physfs/Filesystem.h
	src/modules/filesystem/physfs/PhysfsIo.h
	src/modules/filesystem/physfs/PhysfsIo.cpp
)

set(LOVE_SRC_MODULE_FILESYSTEM
	${LOVE_SRC_MODULE_FILESYSTEM_ROOT}
	${LOVE_SRC_MODULE_FILESYSTEM_PHYSFS}
)

source_group("modules\\filesystem" FILES ${LOVE_SRC_MODULE_FILESYSTEM_ROOT})
source_group("modules\\filesystem\\physfs" FILES ${LOVE_SRC_MODULE_FILESYSTEM_PHYSFS})

#
# love.font
#

set(LOVE_SRC_MODULE_FONT_ROOT
	src/modules/font/BMFontRasterizer.cpp
	src/modules/font/BMFontRasterizer.h
	src/modules/font/Font.cpp
	src/modules/font/Font.h
	src/modules/font/GlyphData.cpp
	src/modules/font/GlyphData.h
	src/modules/font/ImageRasterizer.cpp
	src/modules/font/ImageRasterizer.h
	src/modules/font/Rasterizer.cpp
	src/modules/font/Rasterizer.h
	src/modules/font/TrueTypeRasterizer.cpp
	src/modules/font/TrueTypeRasterizer.h
	src/modules/font/wrap_Font.cpp
	src/modules/font/wrap_Font.h
	src/modules/font/wrap_GlyphData.cpp
	src/modules/font/wrap_GlyphData.h
	src/modules/font/wrap_Rasterizer.cpp
	src/modules/font/wrap_Rasterizer.h
)

set(LOVE_SRC_MODULE_FONT_FREETYPE
	src/modules/font/freetype/Font.cpp
	src/modules/font/freetype/Font.h
	src/modules/font/freetype/TrueTypeRasterizer.cpp
	src/modules/font/freetype/TrueTypeRasterizer.h
)

set(LOVE_SRC_MODULE_FONT
	${LOVE_SRC_MODULE_FONT_ROOT}
	${LOVE_SRC_MODULE_FONT_FREETYPE}
)

source_group("modules\\font" FILES ${LOVE_SRC_MODULE_FONT_ROOT})
source_group("modules\\font\\freetype" FILES ${LOVE_SRC_MODULE_FONT_FREETYPE})

#
# love.graphics
#

set(LOVE_SRC_MODULE_GRAPHICS_ROOT
	src/modules/graphics/Buffer.cpp
	src/modules/graphics/Buffer.h
	src/modules/graphics/Canvas.cpp
	src/modules/graphics/Canvas.h
	src/modules/graphics/depthstencil.cpp
	src/modules/graphics/depthstencil.h
	src/modules/graphics/Deprecations.cpp
	src/modules/graphics/Deprecations.h
	src/modules/graphics/Drawable.cpp
	src/modules/graphics/Drawable.h
	src/modules/graphics/Font.cpp
	src/modules/graphics/Font.h
	src/modules/graphics/Graphics.cpp
	src/modules/graphics/Graphics.h
	src/modules/graphics/Image.cpp
	src/modules/graphics/Image.h
	src/modules/graphics/Mesh.cpp
	src/modules/graphics/Mesh.h
	src/modules/graphics/ParticleSystem.cpp
	src/modules/graphics/ParticleSystem.h
	src/modules/graphics/Polyline.cpp
	src/modules/graphics/Polyline.h
	src/modules/graphics/Quad.cpp
	src/modules/graphics/Quad.h
	src/modules/graphics/Resource.h
	src/modules/graphics/Shader.cpp
	src/modules/graphics/Shader.h
	src/modules/graphics/ShaderStage.cpp
	src/modules/graphics/ShaderStage.h
	src/modules/graphics/SpriteBatch.cpp
	src/modules/graphics/SpriteBatch.h
	src/modules/graphics/StreamBuffer.cpp
	src/modules/graphics/StreamBuffer.h
	src/modules/graphics/Text.cpp
	src/modules/graphics/Text.h
	src/modules/graphics/Texture.cpp
	src/modules/graphics/Texture.h
	src/modules/graphics/vertex.cpp
	src/modules/graphics/vertex.h
	src/modules/graphics/Video.cpp
	src/modules/graphics/Video.h
	src/modules/graphics/Volatile.cpp
	src/modules/graphics/Volatile.h
	src/modules/graphics/wrap_Canvas.cpp
	src/modules/graphics/wrap_Canvas.h
	src/modules/graphics/wrap_Font.cpp
	src/modules/graphics/wrap_Font.h
	src/modules/graphics/wrap_Graphics.cpp
	src/modules/graphics/wrap_Graphics.h
	src/modules/graphics/wrap_Image.cpp
	src/modules/graphics/wrap_Image.h
	src/modules/graphics/wrap_Mesh.cpp
	src/modules/graphics/wrap_Mesh.h
	src/modules/graphics/wrap_ParticleSystem.cpp
	src/modules/graphics/wrap_ParticleSystem.h
	src/modules/graphics/wrap_Quad.cpp
	src/modules/graphics/wrap_Quad.h
	src/modules/graphics/wrap_Shader.cpp
	src/modules/graphics/wrap_Shader.h
	src/modules/graphics/wrap_SpriteBatch.cpp
	src/modules/graphics/wrap_SpriteBatch.h
	src/modules/graphics/wrap_Texture.cpp
	src/modules/graphics/wrap_Texture.h
	src/modules/graphics/wrap_Text.cpp
	src/modules/graphics/wrap_Text.h
	src/modules/graphics/wrap_Video.cpp
	src/modules/graphics/wrap_Video.h
)

set(LOVE_SRC_MODULE_GRAPHICS_OPENGL
	src/modules/graphics/opengl/Buffer.cpp
	src/modules/graphics/opengl/Buffer.h
	src/modules/graphics/opengl/Canvas.cpp
	src/modules/graphics/opengl/Canvas.h
	src/modules/graphics/opengl/FenceSync.cpp
	src/modules/graphics/opengl/FenceSync.h
	src/modules/graphics/opengl/Graphics.cpp
	src/modules/graphics/opengl/Graphics.h
	src/modules/graphics/opengl/Image.cpp
	src/modules/graphics/opengl/Image.h
	src/modules/graphics/opengl/OpenGL.cpp
	src/modules/graphics/opengl/OpenGL.h
	src/modules/graphics/opengl/Shader.cpp
	src/modules/graphics/opengl/Shader.h
	src/modules/graphics/opengl/ShaderStage.cpp
	src/modules/graphics/opengl/ShaderStage.h
	src/modules/graphics/opengl/StreamBuffer.cpp
	src/modules/graphics/opengl/StreamBuffer.h
)

set(LOVE_SRC_MODULE_GRAPHICS
	${LOVE_SRC_MODULE_GRAPHICS_ROOT}
	${LOVE_SRC_MODULE_GRAPHICS_OPENGL}
)

source_group("modules\\graphics" FILES ${LOVE_SRC_MODULE_GRAPHICS_ROOT})
source_group("modules\\graphics\\opengl" FILES ${LOVE_SRC_MODULE_GRAPHICS_OPENGL})

#
# love.image
#

set(LOVE_SRC_MODULE_IMAGE_ROOT
	src/modules/image/CompressedImageData.cpp
	src/modules/image/CompressedImageData.h
	src/modules/image/CompressedSlice.cpp
	src/modules/image/CompressedSlice.h
	src/modules/image/FormatHandler.cpp
	src/modules/image/FormatHandler.h
	src/modules/image/Image.cpp
	src/modules/image/Image.h
	src/modules/image/ImageData.cpp
	src/modules/image/ImageData.h
	src/modules/image/ImageDataBase.cpp
	src/modules/image/ImageDataBase.h
	src/modules/image/wrap_CompressedImageData.cpp
	src/modules/image/wrap_CompressedImageData.h
	src/modules/image/wrap_Image.cpp
	src/modules/image/wrap_Image.h
	src/modules/image/wrap_ImageData.cpp
	src/modules/image/wrap_ImageData.h
)

set(LOVE_SRC_MODULE_IMAGE_MAGPIE
	src/modules/image/magpie/ASTCHandler.cpp
	src/modules/image/magpie/ASTCHandler.h
	src/modules/image/magpie/ddsHandler.cpp
	src/modules/image/magpie/ddsHandler.h
	src/modules/image/magpie/EXRHandler.cpp
	src/modules/image/magpie/EXRHandler.h
	src/modules/image/magpie/KTXHandler.cpp
	src/modules/image/magpie/KTXHandler.h
	src/modules/image/magpie/PKMHandler.cpp
	src/modules/image/magpie/PKMHandler.h
	src/modules/image/magpie/PNGHandler.cpp
	src/modules/image/magpie/PNGHandler.h
	src/modules/image/magpie/PVRHandler.cpp
	src/modules/image/magpie/PVRHandler.h
	src/modules/image/magpie/STBHandler.cpp
	src/modules/image/magpie/STBHandler.h
)

set(LOVE_SRC_MODULE_IMAGE
	${LOVE_SRC_MODULE_IMAGE_ROOT}
	${LOVE_SRC_MODULE_IMAGE_MAGPIE}
)

source_group("modules\\image" FILES ${LOVE_SRC_MODULE_IMAGE_ROOT})
source_group("modules\\image\\magpie" FILES ${LOVE_SRC_MODULE_IMAGE_MAGPIE})

#
# love.joystick
#

set(LOVE_SRC_MODULE_JOYSTICK_ROOT
	src/modules/joystick/Joystick.cpp
	src/modules/joystick/Joystick.h
	src/modules/joystick/JoystickModule.h
	src/modules/joystick/wrap_Joystick.cpp
	src/modules/joystick/wrap_Joystick.h
	src/modules/joystick/wrap_JoystickModule.cpp
	src/modules/joystick/wrap_JoystickModule.h
)

set(LOVE_SRC_MODULE_JOYSTICK_SDL
	src/modules/joystick/sdl/Joystick.cpp
	src/modules/joystick/sdl/Joystick.h
	src/modules/joystick/sdl/JoystickModule.cpp
	src/modules/joystick/sdl/JoystickModule.h
)

set(LOVE_SRC_MODULE_JOYSTICK
	${LOVE_SRC_MODULE_JOYSTICK_ROOT}
	${LOVE_SRC_MODULE_JOYSTICK_SDL}
)

source_group("modules\\joystick" FILES ${LOVE_SRC_MODULE_JOYSTICK_ROOT})
source_group("modules\\joystick\\sdl" FILES ${LOVE_SRC_MODULE_JOYSTICK_SDL})

#
# love.keyboard
#

set(LOVE_SRC_MODULE_KEYBOARD_ROOT
	src/modules/keyboard/Keyboard.cpp
	src/modules/keyboard/Keyboard.h
	src/modules/keyboard/wrap_Keyboard.cpp
	src/modules/keyboard/wrap_Keyboard.h
)

set(LOVE_SRC_MODULE_KEYBOARD_SDL
	src/modules/keyboard/sdl/Keyboard.cpp
	src/modules/keyboard/sdl/Keyboard.h
)

set(LOVE_SRC_MODULE_KEYBOARD
	${LOVE_SRC_MODULE_KEYBOARD_ROOT}
	${LOVE_SRC_MODULE_KEYBOARD_SDL}
)

source_group("modules\\keyboard" FILES ${LOVE_SRC_MODULE_KEYBOARD_ROOT})
source_group("modules\\keyboard\\sdl" FILES ${LOVE_SRC_MODULE_KEYBOARD_SDL})

#
# love.math
#

set(LOVE_SRC_MODULE_MATH
	src/modules/math/BezierCurve.cpp
	src/modules/math/BezierCurve.h
	src/modules/math/MathModule.cpp
	src/modules/math/MathModule.h
	src/modules/math/RandomGenerator.cpp
	src/modules/math/RandomGenerator.h
	src/modules/math/Transform.cpp
	src/modules/math/Transform.h
	src/modules/math/wrap_BezierCurve.cpp
	src/modules/math/wrap_BezierCurve.h
	src/modules/math/wrap_Math.cpp
	src/modules/math/wrap_Math.h
	src/modules/math/wrap_RandomGenerator.cpp
	src/modules/math/wrap_RandomGenerator.h
	src/modules/math/wrap_Transform.cpp
	src/modules/math/wrap_Transform.h
)

source_group("modules\\math" FILES ${LOVE_SRC_MODULE_MATH})

#
# love (module)
#
set(LOVE_SRC_MODULE_LOVE
	src/modules/love/love.cpp
	src/modules/love/love.h
)

source_group("modules\\love" FILES ${LOVE_SRC_MODULE_LOVE})

#
# love.mouse
#

set(LOVE_SRC_MODULE_MOUSE_ROOT
	src/modules/mouse/Cursor.cpp
	src/modules/mouse/Cursor.h
	src/modules/mouse/Mouse.h
	src/modules/mouse/wrap_Cursor.cpp
	src/modules/mouse/wrap_Cursor.h
	src/modules/mouse/wrap_Mouse.cpp
	src/modules/mouse/wrap_Mouse.h
)

set(LOVE_SRC_MODULE_MOUSE_SDL
	src/modules/mouse/sdl/Cursor.cpp
	src/modules/mouse/sdl/Cursor.h
	src/modules/mouse/sdl/Mouse.cpp
	src/modules/mouse/sdl/Mouse.h
)

set(LOVE_SRC_MODULE_MOUSE
	${LOVE_SRC_MODULE_MOUSE_ROOT}
	${LOVE_SRC_MODULE_MOUSE_SDL}
)

source_group("modules\\mouse" FILES ${LOVE_SRC_MODULE_MOUSE_ROOT})
source_group("modules\\mouse\\sdl" FILES ${LOVE_SRC_MODULE_MOUSE_SDL})

#
# love.physics
#

set(LOVE_SRC_MODULE_PHYSICS_ROOT
	src/modules/physics/Body.cpp
	src/modules/physics/Body.h
	src/modules/physics/Joint.cpp
	src/modules/physics/Joint.h
	src/modules/physics/Shape.cpp
	src/modules/physics/Shape.h
)

set(LOVE_SRC_MODULE_PHYSICS_BOX2D
	src/modules/physics/box2d/Body.cpp
	src/modules/physics/box2d/Body.h
	src/modules/physics/box2d/ChainShape.cpp
	src/modules/physics/box2d/ChainShape.h
	src/modules/physics/box2d/CircleShape.cpp
	src/modules/physics/box2d/CircleShape.h
	src/modules/physics/box2d/Contact.cpp
	src/modules/physics/box2d/Contact.h
	src/modules/physics/box2d/DistanceJoint.cpp
	src/modules/physics/box2d/DistanceJoint.h
	src/modules/physics/box2d/EdgeShape.cpp
	src/modules/physics/box2d/EdgeShape.h
	src/modules/physics/box2d/Fixture.cpp
	src/modules/physics/box2d/Fixture.h
	src/modules/physics/box2d/FrictionJoint.cpp
	src/modules/physics/box2d/FrictionJoint.h
	src/modules/physics/box2d/GearJoint.cpp
	src/modules/physics/box2d/GearJoint.h
	src/modules/physics/box2d/Joint.cpp
	src/modules/physics/box2d/Joint.h
	src/modules/physics/box2d/MotorJoint.cpp
	src/modules/physics/box2d/MotorJoint.h
	src/modules/physics/box2d/MouseJoint.cpp
	src/modules/physics/box2d/MouseJoint.h
	src/modules/physics/box2d/Physics.cpp
	src/modules/physics/box2d/Physics.h
	src/modules/physics/box2d/PolygonShape.cpp
	src/modules/physics/box2d/PolygonShape.h
	src/modules/physics/box2d/PrismaticJoint.cpp
	src/modules/physics/box2d/PrismaticJoint.h
	src/modules/physics/box2d/PulleyJoint.cpp
	src/modules/physics/box2d/PulleyJoint.h
	src/modules/physics/box2d/RevoluteJoint.cpp
	src/modules/physics/box2d/RevoluteJoint.h
	src/modules/physics/box2d/RopeJoint.cpp
	src/modules/physics/box2d/RopeJoint.h
	src/modules/physics/box2d/Shape.cpp
	src/modules/physics/box2d/Shape.h
	src/modules/physics/box2d/WeldJoint.cpp
	src/modules/physics/box2d/WeldJoint.h
	src/modules/physics/box2d/WheelJoint.cpp
	src/modules/physics/box2d/WheelJoint.h
	src/modules/physics/box2d/World.cpp
	src/modules/physics/box2d/World.h
	src/modules/physics/box2d/wrap_Body.cpp
	src/modules/physics/box2d/wrap_Body.h
	src/modules/physics/box2d/wrap_ChainShape.cpp
	src/modules/physics/box2d/wrap_ChainShape.h
	src/modules/physics/box2d/wrap_CircleShape.cpp
	src/modules/physics/box2d/wrap_CircleShape.h
	src/modules/physics/box2d/wrap_Contact.cpp
	src/modules/physics/box2d/wrap_Contact.h
	src/modules/physics/box2d/wrap_DistanceJoint.cpp
	src/modules/physics/box2d/wrap_DistanceJoint.h
	src/modules/physics/box2d/wrap_EdgeShape.cpp
	src/modules/physics/box2d/wrap_EdgeShape.h
	src/modules/physics/box2d/wrap_Fixture.cpp
	src/modules/physics/box2d/wrap_Fixture.h
	src/modules/physics/box2d/wrap_FrictionJoint.cpp
	src/modules/physics/box2d/wrap_FrictionJoint.h
	src/modules/physics/box2d/wrap_GearJoint.cpp
	src/modules/physics/box2d/wrap_GearJoint.h
	src/modules/physics/box2d/wrap_Joint.cpp
	src/modules/physics/box2d/wrap_Joint.h
	src/modules/physics/box2d/wrap_MotorJoint.cpp
	src/modules/physics/box2d/wrap_MotorJoint.h
	src/modules/physics/box2d/wrap_MouseJoint.cpp
	src/modules/physics/box2d/wrap_MouseJoint.h
	src/modules/physics/box2d/wrap_Physics.cpp
	src/modules/physics/box2d/wrap_Physics.h
	src/modules/physics/box2d/wrap_PolygonShape.cpp
	src/modules/physics/box2d/wrap_PolygonShape.h
	src/modules/physics/box2d/wrap_PrismaticJoint.cpp
	src/modules/physics/box2d/wrap_PrismaticJoint.h
	src/modules/physics/box2d/wrap_PulleyJoint.cpp
	src/modules/physics/box2d/wrap_PulleyJoint.h
	src/modules/physics/box2d/wrap_RevoluteJoint.cpp
	src/modules/physics/box2d/wrap_RevoluteJoint.h
	src/modules/physics/box2d/wrap_RopeJoint.cpp
	src/modules/physics/box2d/wrap_RopeJoint.h
	src/modules/physics/box2d/wrap_Shape.cpp
	src/modules/physics/box2d/wrap_Shape.h
	src/modules/physics/box2d/wrap_WeldJoint.cpp
	src/modules/physics/box2d/wrap_WeldJoint.h
	src/modules/physics/box2d/wrap_WheelJoint.cpp
	src/modules/physics/box2d/wrap_WheelJoint.h
	src/modules/physics/box2d/wrap_World.cpp
	src/modules/physics/box2d/wrap_World.h
)

set(LOVE_SRC_MODULE_PHYSICS
	${LOVE_SRC_MODULE_PHYSICS_ROOT}
	${LOVE_SRC_MODULE_PHYSICS_BOX2D}
)

source_group("modules\\physics" FILES ${LOVE_SRC_MODULE_PHYSICS_ROOT})
source_group("modules\\physics\\box2d" FILES ${LOVE_SRC_MODULE_PHYSICS_BOX2D})

#
# love.sound
#

set(LOVE_SRC_MODULE_SOUND_ROOT
	src/modules/sound/Decoder.cpp
	src/modules/sound/Decoder.h
	src/modules/sound/Sound.cpp
	src/modules/sound/Sound.h
	src/modules/sound/SoundData.cpp
	src/modules/sound/SoundData.h
	src/modules/sound/wrap_Decoder.cpp
	src/modules/sound/wrap_Decoder.h
	src/modules/sound/wrap_Sound.cpp
	src/modules/sound/wrap_Sound.h
	src/modules/sound/wrap_SoundData.cpp
	src/modules/sound/wrap_SoundData.h
)

set(LOVE_SRC_MODULE_SOUND_LULLABY
	src/modules/sound/lullaby/FLACDecoder.cpp
	src/modules/sound/lullaby/FLACDecoder.h
	src/modules/sound/lullaby/GmeDecoder.cpp
	src/modules/sound/lullaby/GmeDecoder.h
	src/modules/sound/lullaby/ModPlugDecoder.cpp
	src/modules/sound/lullaby/ModPlugDecoder.h
	src/modules/sound/lullaby/Sound.cpp
	src/modules/sound/lullaby/Sound.h
	src/modules/sound/lullaby/VorbisDecoder.cpp
	src/modules/sound/lullaby/VorbisDecoder.h
	src/modules/sound/lullaby/WaveDecoder.cpp
	src/modules/sound/lullaby/WaveDecoder.h
)

if(LOVE_MPG123)
	set(LOVE_SRC_MODULE_SOUND_LULLABY
		${LOVE_SRC_MODULE_SOUND_LULLABY}
		src/modules/sound/lullaby/Mpg123Decoder.cpp
		src/modules/sound/lullaby/Mpg123Decoder.h
	)
endif()

set(LOVE_SRC_MODULE_SOUND
	${LOVE_SRC_MODULE_SOUND_ROOT}
	${LOVE_SRC_MODULE_SOUND_LULLABY}
)

source_group("modules\\sound" FILES ${LOVE_SRC_MODULE_SOUND_ROOT})
source_group("modules\\sound\\lullaby" FILES ${LOVE_SRC_MODULE_SOUND_LULLABY})

#
# love.system
#

set(LOVE_SRC_MODULE_SYSTEM_ROOT
	src/modules/system/System.cpp
	src/modules/system/System.h
	src/modules/system/wrap_System.cpp
	src/modules/system/wrap_System.h
)

set(LOVE_SRC_MODULE_SYSTEM_SDL
	src/modules/system/sdl/System.cpp
	src/modules/system/sdl/System.h
)

set(LOVE_SRC_MODULE_SYSTEM
	${LOVE_SRC_MODULE_SYSTEM_ROOT}
	${LOVE_SRC_MODULE_SYSTEM_SDL}
)

source_group("modules\\system" FILES ${LOVE_SRC_MODULE_SYSTEM_ROOT})
source_group("modules\\system\\sdl" FILES ${LOVE_SRC_MODULE_SYSTEM_SDL})

#
# love.thread
#

set(LOVE_SRC_MODULE_THREAD_ROOT
	src/modules/thread/Channel.cpp
	src/modules/thread/Channel.h
	src/modules/thread/LuaThread.cpp
	src/modules/thread/LuaThread.h
	src/modules/thread/Thread.h
	src/modules/thread/ThreadModule.cpp
	src/modules/thread/ThreadModule.h
	src/modules/thread/threads.cpp
	src/modules/thread/threads.h
	src/modules/thread/wrap_Channel.cpp
	src/modules/thread/wrap_Channel.h
	src/modules/thread/wrap_LuaThread.cpp
	src/modules/thread/wrap_LuaThread.h
	src/modules/thread/wrap_ThreadModule.cpp
	src/modules/thread/wrap_ThreadModule.h
)

set(LOVE_SRC_MODULE_THREAD_SDL
	src/modules/thread/sdl/Thread.cpp
	src/modules/thread/sdl/Thread.h
	src/modules/thread/sdl/threads.cpp
	src/modules/thread/sdl/threads.h
)

set(LOVE_SRC_MODULE_THREAD
	${LOVE_SRC_MODULE_THREAD_ROOT}
	${LOVE_SRC_MODULE_THREAD_SDL}
)

source_group("modules\\thread" FILES ${LOVE_SRC_MODULE_THREAD_ROOT})
source_group("modules\\thread\\sdl" FILES ${LOVE_SRC_MODULE_THREAD_SDL})

#
# love.timer
#

set(LOVE_SRC_MODULE_TIMER
	src/modules/timer/Timer.cpp
	src/modules/timer/Timer.h
	src/modules/timer/wrap_Timer.cpp
	src/modules/timer/wrap_Timer.h
)

source_group("modules\\timer" FILES ${LOVE_SRC_MODULE_TIMER})

#
# love.touch
#

set(LOVE_SRC_MODULE_TOUCH_ROOT
	src/modules/touch/Touch.h
	src/modules/touch/wrap_Touch.cpp
	src/modules/touch/wrap_Touch.h
)

set(LOVE_SRC_MODULE_TOUCH_SDL
	src/modules/touch/sdl/Touch.cpp
	src/modules/touch/sdl/Touch.h
)

set(LOVE_SRC_MODULE_TOUCH
	${LOVE_SRC_MODULE_TOUCH_ROOT}
	${LOVE_SRC_MODULE_TOUCH_SDL}
)

source_group("modules\\touch" FILES ${LOVE_SRC_MODULE_TOUCH_ROOT})
source_group("modules\\touch\\sdl" FILES ${LOVE_SRC_MODULE_TOUCH_SDL})

#
# love.video
#

set(LOVE_SRC_MODULE_VIDEO_ROOT
	src/modules/video/Video.h
	src/modules/video/VideoStream.cpp
	src/modules/video/VideoStream.h
	src/modules/video/wrap_Video.cpp
	src/modules/video/wrap_Video.h
	src/modules/video/wrap_VideoStream.cpp
	src/modules/video/wrap_VideoStream.h
)

set(LOVE_SRC_MODULE_VIDEO_THEORA
	src/modules/video/theora/Video.cpp
	src/modules/video/theora/Video.h
	src/modules/video/theora/OggDemuxer.cpp
	src/modules/video/theora/OggDemuxer.h
	src/modules/video/theora/TheoraVideoStream.cpp
	src/modules/video/theora/TheoraVideoStream.h
)

set(LOVE_SRC_MODULE_VIDEO
	${LOVE_SRC_MODULE_VIDEO_ROOT}
	${LOVE_SRC_MODULE_VIDEO_THEORA}
)

source_group("modules\\video" FILES ${LOVE_SRC_MODULE_VIDEO_ROOT})
source_group("modules\\video\\theora" FILES ${LOVE_SRC_MODULE_VIDEO_THEORA})

#
# love.window
#

set(LOVE_SRC_MODULE_WINDOW_ROOT
	src/modules/window/Window.cpp
	src/modules/window/Window.h
	src/modules/window/wrap_Window.cpp
	src/modules/window/wrap_Window.h
)

set(LOVE_SRC_MODULE_WINDOW_SDL
	src/modules/window/sdl/Window.cpp
	src/modules/window/sdl/Window.h
)

set(LOVE_SRC_MODULE_WINDOW
	${LOVE_SRC_MODULE_WINDOW_ROOT}
	${LOVE_SRC_MODULE_WINDOW_SDL}
)

source_group("modules\\window" FILES ${LOVE_SRC_MODULE_WINDOW_ROOT})
source_group("modules\\window\\sdl" FILES ${LOVE_SRC_MODULE_WINDOW_SDL})

###################################
# Third-party libraries
###################################

#
# Box2D
#

set(LOVE_SRC_3P_BOX2D_ROOT
	src/libraries/Box2D/Box2D.h
)

set(LOVE_SRC_3P_BOX2D_COLLISION
	src/libraries/Box2D/Collision/b2BroadPhase.cpp
	src/libraries/Box2D/Collision/b2BroadPhase.h
	src/libraries/Box2D/Collision/b2CollideCircle.cpp
	src/libraries/Box2D/Collision/b2CollideEdge.cpp
	src/libraries/Box2D/Collision/b2CollidePolygon.cpp
	src/libraries/Box2D/Collision/b2Collision.cpp
	src/libraries/Box2D/Collision/b2Collision.h
	src/libraries/Box2D/Collision/b2Distance.cpp
	src/libraries/Box2D/Collision/b2Distance.h
	src/libraries/Box2D/Collision/b2DynamicTree.cpp
	src/libraries/Box2D/Collision/b2DynamicTree.h
	src/libraries/Box2D/Collision/b2TimeOfImpact.cpp
	src/libraries/Box2D/Collision/b2TimeOfImpact.h
)

set(LOVE_SRC_3P_BOX2D_COLLISION_SHAPES
	src/libraries/Box2D/Collision/Shapes/b2ChainShape.cpp
	src/libraries/Box2D/Collision/Shapes/b2ChainShape.h
	src/libraries/Box2D/Collision/Shapes/b2CircleShape.cpp
	src/libraries/Box2D/Collision/Shapes/b2CircleShape.h
	src/libraries/Box2D/Collision/Shapes/b2EdgeShape.cpp
	src/libraries/Box2D/Collision/Shapes/b2EdgeShape.h
	src/libraries/Box2D/Collision/Shapes/b2PolygonShape.cpp
	src/libraries/Box2D/Collision/Shapes/b2PolygonShape.h
	src/libraries/Box2D/Collision/Shapes/b2Shape.h
)

set(LOVE_SRC_3P_BOX2D_COMMON
	src/libraries/Box2D/Common/b2BlockAllocator.cpp
	src/libraries/Box2D/Common/b2BlockAllocator.h
	src/libraries/Box2D/Common/b2Draw.cpp
	src/libraries/Box2D/Common/b2Draw.h
	src/libraries/Box2D/Common/b2GrowableStack.h
	src/libraries/Box2D/Common/b2Math.cpp
	src/libraries/Box2D/Common/b2Math.h
	src/libraries/Box2D/Common/b2Settings.cpp
	src/libraries/Box2D/Common/b2Settings.h
	src/libraries/Box2D/Common/b2StackAllocator.cpp
	src/libraries/Box2D/Common/b2StackAllocator.h
	src/libraries/Box2D/Common/b2Timer.cpp
	src/libraries/Box2D/Common/b2Timer.h
)

set(LOVE_SRC_3P_BOX2D_DYNAMICS
	src/libraries/Box2D/Dynamics/b2Body.cpp
	src/libraries/Box2D/Dynamics/b2Body.h
	src/libraries/Box2D/Dynamics/b2ContactManager.cpp
	src/libraries/Box2D/Dynamics/b2ContactManager.h
	src/libraries/Box2D/Dynamics/b2Fixture.cpp
	src/libraries/Box2D/Dynamics/b2Fixture.h
	src/libraries/Box2D/Dynamics/b2Island.cpp
	src/libraries/Box2D/Dynamics/b2Island.h
	src/libraries/Box2D/Dynamics/b2TimeStep.h
	src/libraries/Box2D/Dynamics/b2World.cpp
	src/libraries/Box2D/Dynamics/b2World.h
	src/libraries/Box2D/Dynamics/b2WorldCallbacks.cpp
	src/libraries/Box2D/Dynamics/b2WorldCallbacks.h
)

set(LOVE_SRC_3P_BOX2D_DYNAMICS_CONTACTS
	src/libraries/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2ChainAndCircleContact.h
	src/libraries/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.h
	src/libraries/Box2D/Dynamics/Contacts/b2CircleContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2CircleContact.h
	src/libraries/Box2D/Dynamics/Contacts/b2Contact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2Contact.h
	src/libraries/Box2D/Dynamics/Contacts/b2ContactSolver.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2ContactSolver.h
	src/libraries/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.h
	src/libraries/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.h
	src/libraries/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.h
	src/libraries/Box2D/Dynamics/Contacts/b2PolygonContact.cpp
	src/libraries/Box2D/Dynamics/Contacts/b2PolygonContact.h
)

set(LOVE_SRC_3P_BOX2D_DYNAMICS_JOINTS
	src/libraries/Box2D/Dynamics/Joints/b2DistanceJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2DistanceJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2FrictionJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2FrictionJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2GearJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2GearJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2Joint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2Joint.h
	src/libraries/Box2D/Dynamics/Joints/b2MotorJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2MotorJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2MouseJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2MouseJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2PrismaticJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2PrismaticJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2PulleyJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2PulleyJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2RevoluteJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2RevoluteJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2RopeJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2RopeJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2WeldJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2WeldJoint.h
	src/libraries/Box2D/Dynamics/Joints/b2WheelJoint.cpp
	src/libraries/Box2D/Dynamics/Joints/b2WheelJoint.h
)

set(LOVE_SRC_3P_BOX2D_ROPE
	src/libraries/Box2D/Rope/b2Rope.cpp
	src/libraries/Box2D/Rope/b2Rope.h
)

set(LOVE_SRC_3P_BOX2D
	${LOVE_SRC_3P_BOX2D_ROOT}
	${LOVE_SRC_3P_BOX2D_COLLISION}
	${LOVE_SRC_3P_BOX2D_COLLISION_SHAPES}
	${LOVE_SRC_3P_BOX2D_COMMON}
	${LOVE_SRC_3P_BOX2D_DYNAMICS}
	${LOVE_SRC_3P_BOX2D_DYNAMICS_CONTACTS}
	${LOVE_SRC_3P_BOX2D_DYNAMICS_JOINTS}
	${LOVE_SRC_3P_BOX2D_ROPE}
)

add_library(love_3p_box2d ${LOVE_SRC_3P_BOX2D})

#
# ddsparse
#

set(LOVE_SRC_3P_DDSPARSE
	src/libraries/ddsparse/ddsinfo.h
	src/libraries/ddsparse/ddsparse.cpp
	src/libraries/ddsparse/ddsparse.h
)

add_library(love_3p_ddsparse ${LOVE_SRC_3P_DDSPARSE})

#
# dr_flac
#

set(LOVE_SRC_3P_DRFLAC
	src/libraries/dr_flac/dr_flac.h
)

# dr_flac has no implementation files of its own.

#
# enet
#

set(LOVE_SRC_3P_ENET_ROOT
	src/libraries/enet/enet.cpp
	src/libraries/enet/lua-enet.h
)

set(LOVE_SRC_3P_ENET_LIBENET
	src/libraries/enet/libenet/callbacks.c
	src/libraries/enet/libenet/compress.c
	src/libraries/enet/libenet/host.c
	src/libraries/enet/libenet/list.c
	src/libraries/enet/libenet/packet.c
	src/libraries/enet/libenet/peer.c
	src/libraries/enet/libenet/protocol.c
	src/libraries/enet/libenet/unix.c
	src/libraries/enet/libenet/win32.c
)

set(LOVE_SRC_3P_ENET_LIBENET_INCLUDE_ENET
	src/libraries/enet/libenet/include/enet/enet.h
	src/libraries/enet/libenet/include/enet/list.h
	src/libraries/enet/libenet/include/enet/protocol.h
	src/libraries/enet/libenet/include/enet/time.h
	src/libraries/enet/libenet/include/enet/types.h
	src/libraries/enet/libenet/include/enet/unix.h
	src/libraries/enet/libenet/include/enet/utility.h
	src/libraries/enet/libenet/include/enet/win32.h
)

set(LOVE_SRC_3P_ENET
	${LOVE_SRC_3P_ENET_ROOT}
	${LOVE_SRC_3P_ENET_LIBENET}
	${LOVE_SRC_3P_ENET_LIBENET_INCLUDE_ENET}
)

add_library(love_3p_enet ${LOVE_SRC_3P_ENET})
target_link_libraries(love_3p_enet ${LOVE_LUA_LIBRARY})
target_include_directories(love_3p_enet PUBLIC src/libraries/enet/libenet/include)
if(MINGW)
	target_link_libraries(love_3p_enet winmm.a)
endif()

#
# GLAD
#

set(LOVE_SRC_3P_GLAD
	src/libraries/glad/glad.cpp
	src/libraries/glad/glad.hpp
	src/libraries/glad/gladfuncs.hpp
)

add_library(love_3p_glad ${LOVE_SRC_3P_GLAD})

#
# glslang
#

set(LOVE_SRC_3P_GLSLANG_GLSLANG_GENERICCODEGEN
	src/libraries/glslang/glslang/GenericCodeGen/CodeGen.cpp
	src/libraries/glslang/glslang/GenericCodeGen/Link.cpp
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_INCLUDE
	src/libraries/glslang/glslang/Include/arrays.h
	src/libraries/glslang/glslang/Include/BaseTypes.h
	src/libraries/glslang/glslang/Include/Common.h
	src/libraries/glslang/glslang/Include/ConstantUnion.h
	src/libraries/glslang/glslang/Include/InfoSink.h
	src/libraries/glslang/glslang/Include/InitializeGlobals.h
	src/libraries/glslang/glslang/Include/intermediate.h
	src/libraries/glslang/glslang/Include/PoolAlloc.h
	src/libraries/glslang/glslang/Include/ResourceLimits.h
	src/libraries/glslang/glslang/Include/revision.h
	src/libraries/glslang/glslang/Include/ShHandle.h
	src/libraries/glslang/glslang/Include/Types.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT_PREPROCESSOR
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpContext.h
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpTokens.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT
	${LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT_PREPROCESSOR}
	src/libraries/glslang/glslang/MachineIndependent/attribute.cpp
	src/libraries/glslang/glslang/MachineIndependent/attribute.h
	src/libraries/glslang/glslang/MachineIndependent/Constant.cpp
	src/libraries/glslang/glslang/MachineIndependent/gl_types.h
	src/libraries/glslang/glslang/MachineIndependent/glslang_tab.cpp
	src/libraries/glslang/glslang/MachineIndependent/glslang_tab.cpp.h
	src/libraries/glslang/glslang/MachineIndependent/InfoSink.cpp
	src/libraries/glslang/glslang/MachineIndependent/Initialize.cpp
	src/libraries/glslang/glslang/MachineIndependent/Initialize.h
	src/libraries/glslang/glslang/MachineIndependent/Intermediate.cpp
	src/libraries/glslang/glslang/MachineIndependent/intermOut.cpp
	src/libraries/glslang/glslang/MachineIndependent/IntermTraverse.cpp
	src/libraries/glslang/glslang/MachineIndependent/iomapper.cpp
	src/libraries/glslang/glslang/MachineIndependent/iomapper.h
	src/libraries/glslang/glslang/MachineIndependent/limits.cpp
	src/libraries/glslang/glslang/MachineIndependent/linkValidate.cpp
	src/libraries/glslang/glslang/MachineIndependent/LiveTraverser.h
	src/libraries/glslang/glslang/MachineIndependent/localintermediate.h
	src/libraries/glslang/glslang/MachineIndependent/parseConst.cpp
	src/libraries/glslang/glslang/MachineIndependent/ParseContextBase.cpp
	src/libraries/glslang/glslang/MachineIndependent/ParseHelper.cpp
	src/libraries/glslang/glslang/MachineIndependent/ParseHelper.h
	src/libraries/glslang/glslang/MachineIndependent/parseVersions.h
	src/libraries/glslang/glslang/MachineIndependent/pch.cpp
	src/libraries/glslang/glslang/MachineIndependent/pch.h
	src/libraries/glslang/glslang/MachineIndependent/PoolAlloc.cpp
	src/libraries/glslang/glslang/MachineIndependent/propagateNoContraction.cpp
	src/libraries/glslang/glslang/MachineIndependent/propagateNoContraction.h
	src/libraries/glslang/glslang/MachineIndependent/reflection.cpp
	src/libraries/glslang/glslang/MachineIndependent/reflection.h
	src/libraries/glslang/glslang/MachineIndependent/RemoveTree.cpp
	src/libraries/glslang/glslang/MachineIndependent/RemoveTree.h
	src/libraries/glslang/glslang/MachineIndependent/Scan.cpp
	src/libraries/glslang/glslang/MachineIndependent/Scan.h
	src/libraries/glslang/glslang/MachineIndependent/ScanContext.h
	src/libraries/glslang/glslang/MachineIndependent/ShaderLang.cpp
	src/libraries/glslang/glslang/MachineIndependent/SymbolTable.cpp
	src/libraries/glslang/glslang/MachineIndependent/SymbolTable.h
	src/libraries/glslang/glslang/MachineIndependent/Versions.cpp
	src/libraries/glslang/glslang/MachineIndependent/Versions.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT
	src/libraries/glslang/glslang/OSDependent/osinclude.h
)

if(MSVC OR MINGW)
	set(LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT
		${LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT}
		src/libraries/glslang/glslang/OSDependent/Windows/main.cpp
		src/libraries/glslang/glslang/OSDependent/Windows/ossource.cpp
	)
else()
	set(LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT
		${LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT}
		src/libraries/glslang/glslang/OSDependent/Unix/ossource.cpp
	)
endif()

set(LOVE_SRC_3P_GLSLANG_GLSLANG_PUBLIC
	src/libraries/glslang/glslang/Public/ShaderLang.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG
	${LOVE_SRC_3P_GLSLANG_GLSLANG_GENERICCODEGEN}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_INCLUDE}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_PUBLIC}
)

set(LOVE_SRC_3P_GLSLANG_OGLCOMPILERSDLL
	src/libraries/glslang/OGLCompilersDLL/InitializeDll.cpp
	src/libraries/glslang/OGLCompilersDLL/InitializeDll.h
)

set(LOVE_SRC_3P_GLSLANG
	${LOVE_SRC_3P_GLSLANG_GLSLANG}
	${LOVE_SRC_3P_GLSLANG_OGLCOMPILERSDLL}
)

add_library(love_3p_glslang ${LOVE_SRC_3P_GLSLANG})

#
# LodePNG
#

set(LOVE_SRC_3P_LODEPNG
	src/libraries/lodepng/lodepng.cpp
	src/libraries/lodepng/lodepng.h
)

add_library(love_3p_lodepng ${LOVE_SRC_3P_LODEPNG})

#
# luasocket
#

set(LOVE_SRC_3P_LUASOCKET_ROOT
	src/libraries/luasocket/luasocket.cpp
	src/libraries/luasocket/luasocket.h
)

set(LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET
	src/libraries/luasocket/libluasocket/auxiliar.c
	src/libraries/luasocket/libluasocket/auxiliar.h
	src/libraries/luasocket/libluasocket/buffer.c
	src/libraries/luasocket/libluasocket/buffer.h
	src/libraries/luasocket/libluasocket/compat.c
	src/libraries/luasocket/libluasocket/compat.h
	src/libraries/luasocket/libluasocket/except.c
	src/libraries/luasocket/libluasocket/except.h
	src/libraries/luasocket/libluasocket/ftp.lua.h
	src/libraries/luasocket/libluasocket/headers.lua.h
	src/libraries/luasocket/libluasocket/http.lua.h
	src/libraries/luasocket/libluasocket/inet.c
	src/libraries/luasocket/libluasocket/inet.h
	src/libraries/luasocket/libluasocket/io.c
	src/libraries/luasocket/libluasocket/io.h
	src/libraries/luasocket/libluasocket/ltn12.lua.h
	src/libraries/luasocket/libluasocket/luasocket.c
	src/libraries/luasocket/libluasocket/luasocket.h
	src/libraries/luasocket/libluasocket/mbox.lua.h
	src/libraries/luasocket/libluasocket/mime.c
	src/libraries/luasocket/libluasocket/mime.h
	src/libraries/luasocket/libluasocket/mime.lua.h
	src/libraries/luasocket/libluasocket/options.c
	src/libraries/luasocket/libluasocket/options.h
	src/libraries/luasocket/libluasocket/pierror.h
	src/libraries/luasocket/libluasocket/select.c
	src/libraries/luasocket/libluasocket/select.h
	src/libraries/luasocket/libluasocket/smtp.lua.h
	src/libraries/luasocket/libluasocket/socket.h
	src/libraries/luasocket/libluasocket/socket.lua.h
	src/libraries/luasocket/libluasocket/tcp.c
	src/libraries/luasocket/libluasocket/tcp.h
	src/libraries/luasocket/libluasocket/timeout.c
	src/libraries/luasocket/libluasocket/timeout.h
	src/libraries/luasocket/libluasocket/tp.lua.h
	src/libraries/luasocket/libluasocket/udp.c
	src/libraries/luasocket/libluasocket/udp.h
	src/libraries/luasocket/libluasocket/url.lua.h
)

set(LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET)

if(MINGW)
	set(WIN32_LIB_EXT .a)
else()
	set(WIN32_LIB_EXT .lib)
endif()

if(MSVC OR MINGW)
	set(LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET
		${LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET}
		src/libraries/luasocket/libluasocket/wsocket.c
		src/libraries/luasocket/libluasocket/wsocket.h
	)

	set(LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET
		${LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET}
		ws2_32${WIN32_LIB_EXT}
	)
else()
	set(LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET
		${LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET}
		src/libraries/luasocket/libluasocket/serial.c
		src/libraries/luasocket/libluasocket/unix.c
		src/libraries/luasocket/libluasocket/unix.h
		src/libraries/luasocket/libluasocket/unixtcp.c
		src/libraries/luasocket/libluasocket/unixtcp.h
		src/libraries/luasocket/libluasocket/unixudp.c
		src/libraries/luasocket/libluasocket/unixudp.h
		src/libraries/luasocket/libluasocket/usocket.c
		src/libraries/luasocket/libluasocket/usocket.h
	)
endif()

set(LOVE_SRC_3P_LUASOCKET
	${LOVE_SRC_3P_LUASOCKET_ROOT}
	${LOVE_SRC_3P_LUASOCKET_LIBLUASOCKET}
)

add_library(love_3p_luasocket ${LOVE_SRC_3P_LUASOCKET})
target_link_libraries(love_3p_luasocket ${LOVE_LUA_LIBRARY} ${LOVE_LINK_L3P_LUASOCKET_LIBLUASOCKET})

#
# APIs from Lua 5.3
#

set(LOVE_SRC_3P_LUA53
	src/libraries/lua53/lprefix.h
	src/libraries/lua53/lstrlib.c
	src/libraries/lua53/lstrlib.h
	src/libraries/lua53/lutf8lib.c
	src/libraries/lua53/lutf8lib.h
)

add_library(love_3p_lua53 ${LOVE_SRC_3P_LUA53})
target_link_libraries(love_3p_lua53 ${LOVE_LUA_LIBRARY})

#
# lz4
#

set(LOVE_SRC_3P_LZ4
	src/libraries/lz4/lz4.c
	src/libraries/lz4/lz4.h
	src/libraries/lz4/lz4hc.c
	src/libraries/lz4/lz4hc.h
	src/libraries/lz4/lz4opt.h
)

add_library(love_3p_lz4 ${LOVE_SRC_3P_LZ4})

#
# noise1234
#

set(LOVE_SRC_3P_NOISE1234
	src/libraries/noise1234/noise1234.cpp
	src/libraries/noise1234/noise1234.h
	src/libraries/noise1234/simplexnoise1234.cpp
	src/libraries/noise1234/simplexnoise1234.h
)

add_library(love_3p_noise1234 ${LOVE_SRC_3P_NOISE1234})

#
# physfs
#

set(LOVE_SRC_3P_PHYSFS
	src/libraries/physfs/physfs_archiver_7z.c
	src/libraries/physfs/physfs_archiver_dir.c
	src/libraries/physfs/physfs_archiver_grp.c
	src/libraries/physfs/physfs_archiver_hog.c
	src/libraries/physfs/physfs_archiver_iso9660.c
	src/libraries/physfs/physfs_archiver_mvl.c
	src/libraries/physfs/physfs_archiver_qpak.c
	src/libraries/physfs/physfs_archiver_slb.c
	src/libraries/physfs/physfs_archiver_unpacked.c
	src/libraries/physfs/physfs_archiver_vdf.c
	src/libraries/physfs/physfs_archiver_wad.c
	src/libraries/physfs/physfs_archiver_zip.c
	src/libraries/physfs/physfs_byteorder.c
	src/libraries/physfs/physfs_casefolding.h
	src/libraries/physfs/physfs_internal.h
	src/libraries/physfs/physfs_lzmasdk.h
	src/libraries/physfs/physfs_miniz.h
	src/libraries/physfs/physfs_platform_haiku.cpp
	src/libraries/physfs/physfs_platform_os2.c
	src/libraries/physfs/physfs_platform_posix.c
	src/libraries/physfs/physfs_platform_qnx.c
	src/libraries/physfs/physfs_platform_unix.c
	src/libraries/physfs/physfs_platform_windows.c
	src/libraries/physfs/physfs_platform_winrt.cpp
	src/libraries/physfs/physfs_platforms.h
	src/libraries/physfs/physfs_unicode.c
	src/libraries/physfs/physfs.c
	src/libraries/physfs/physfs.h
)

if(APPLE)
	set(LOVE_SRC_3P_PHYSFS ${LOVE_SRC_3P_PHYSFS}
		src/libraries/physfs/physfs_platform_apple.m
	)
	set(LOVE_LINK_LIBRARIES ${LOVE_LINK_LIBRARIES} "-framework IOKit")
endif()

add_library(love_3p_physfs ${LOVE_SRC_3P_PHYSFS})

#
# stb_image
#

set(LOVE_SRC_3P_STB
	src/libraries/stb/stb_image.h
)

# stb_image has no implementation files of its own.

#
# tiny exr
#

set(LOVE_SRC_3P_TINYEXR
	src/libraries/tinyexr/tinyexr.h
)

# tinyexr has no implementation files of its own.

#
# utf8
#

set(LOVE_SRC_3P_UTF8_ROOT src/libraries/utf8/utf8.h)

set(LOVE_SRC_3P_UTF8_UTF8
	src/libraries/utf8/utf8/checked.h
	src/libraries/utf8/utf8/core.h
	src/libraries/utf8/utf8/unchecked.h
)

set(LOVE_SRC_3P_UTF8
	${LOVE_SRC_3P_UTF8_ROOT}
	${LOVE_SRC_3P_UTF8_UTF8}
)

# This library is all headers ... so there is no need to
# add_library() here.

#
# Wuff
#

set(LOVE_SRC_3P_WUFF
	src/libraries/Wuff/wuff.c
	src/libraries/Wuff/wuff.h
	src/libraries/Wuff/wuff_config.h
	src/libraries/Wuff/wuff_convert.c
	src/libraries/Wuff/wuff_convert.h
	src/libraries/Wuff/wuff_internal.c
	src/libraries/Wuff/wuff_internal.h
	src/libraries/Wuff/wuff_memory.c
)

add_library(love_3p_wuff ${LOVE_SRC_3P_WUFF})

#
# xxHash
#

set(LOVE_SRC_3P_XXHASH
	src/libraries/xxHash/xxhash.c
	src/libraries/xxHash/xxhash.h
)

add_library(love_3p_xxhash ${LOVE_SRC_3P_XXHASH})

set(LOVE_3P
	love_3p_box2d
	love_3p_ddsparse
	love_3p_enet
	love_3p_glad
	love_3p_glslang
	love_3p_lodepng
	love_3p_luasocket
	love_3p_lua53
	love_3p_lz4
	love_3p_noise1234
	love_3p_physfs
	love_3p_wuff
	love_3p_xxhash
)

love_disable_warnings(love_3p_box2d love_3p_enet love_3p_luasocket love_3p_physfs)

#
# liblove
#
set(LOVE_LIB_SRC
	${LOVE_SRC_COMMON}
	# Modules
	${LOVE_SRC_MODULE_AUDIO}
	${LOVE_SRC_MODULE_DATA}
	${LOVE_SRC_MODULE_EVENT}
	${LOVE_SRC_MODULE_FILESYSTEM}
	${LOVE_SRC_MODULE_FONT}
	${LOVE_SRC_MODULE_GRAPHICS}
	${LOVE_SRC_MODULE_IMAGE}
	${LOVE_SRC_MODULE_JOYSTICK}
	${LOVE_SRC_MODULE_KEYBOARD}
	${LOVE_SRC_MODULE_LOVE}
	${LOVE_SRC_MODULE_MATH}
	${LOVE_SRC_MODULE_MOUSE}
	${LOVE_SRC_MODULE_PHYSICS}
	${LOVE_SRC_MODULE_SOUND}
	${LOVE_SRC_MODULE_SYSTEM}
	${LOVE_SRC_MODULE_THREAD}
	${LOVE_SRC_MODULE_TIMER}
	${LOVE_SRC_MODULE_TOUCH}
	${LOVE_SRC_MODULE_VIDEO}
	${LOVE_SRC_MODULE_WINDOW}
)

include_directories(
	BEFORE
	src
	src/libraries
	src/modules
	${LOVE_INCLUDE_DIRS}
)

link_directories(${LOVE_LINK_DIRS})

set(LOVE_RC)

if(MSVC OR MINGW)
	set(LOVE_LINK_LIBRARIES ${LOVE_LINK_LIBRARIES}
		ws2_32${WIN32_LIB_EXT}
		winmm${WIN32_LIB_EXT}
		dwmapi${WIN32_LIB_EXT}
	)

	set(LOVE_RC
		extra/windows/love.rc
		extra/windows/love.ico
	)

	if(MINGW)
		# UTF-16 flags passed to windres. windres invokes gcc as preprocessor
		# -> gcc outputs utf8, so windres must read-in codepage 65001 (utf8)
		set(CMAKE_RC_FLAGS ${CMAKE_RC_FLAGS} "-c 65001 --preprocessor-arg=-finput-charset=UTF-16LE")
	endif()
endif()

add_library(${LOVE_LIB_NAME} SHARED ${LOVE_LIB_SRC} ${LOVE_RC})
set_target_properties(${LOVE_LIB_NAME} PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(${LOVE_LIB_NAME} ${LOVE_LINK_LIBRARIES} ${LOVE_3P})

if(LOVE_EXTRA_DEPENDECIES)
	add_dependencies(${LOVE_LIB_NAME} ${LOVE_EXTRA_DEPENDECIES})
endif()

if(MSVC)
	set_target_properties(${LOVE_LIB_NAME} PROPERTIES RELEASE_OUTPUT_NAME "love" PDB_NAME "liblove" IMPORT_PREFIX "lib")
	set_target_properties(${LOVE_LIB_NAME} PROPERTIES DEBUG_OUTPUT_NAME "love" PDB_NAME "liblove" IMPORT_PREFIX "lib")
endif()

#
# love (executable)
#
add_executable(${LOVE_EXE_NAME} WIN32 src/love.cpp ${LOVE_RC})
target_link_libraries(${LOVE_EXE_NAME} ${LOVE_LIB_NAME})
set_target_properties(${LOVE_EXE_NAME} PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)

if(MSVC OR MINGW)
	add_executable(${LOVE_CONSOLE_EXE_NAME} src/love.cpp ${LOVE_RC})
	target_link_libraries(${LOVE_CONSOLE_EXE_NAME} ${LOVE_LIB_NAME})
endif()

function(post_step_move_dll ARG_POST_TARGET ARG_TARGET_OR_FILE)
	if(TARGET ${ARG_TARGET_OR_FILE})
		add_custom_command(TARGET ${ARG_POST_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			$<TARGET_FILE:${ARG_TARGET_OR_FILE}>
			${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/$<TARGET_FILE_NAME:${ARG_TARGET_OR_FILE}>)
	else()
		get_filename_component(TEMP_FILENAME ${ARG_TARGET_OR_FILE} NAME)
		add_custom_command(TARGET ${ARG_POST_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${ARG_TARGET_OR_FILE}
			${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/${TEMP_FILENAME})
	endif()
endfunction()

# Add post build steps to move the DLLs next to the binary. Otherwise
# running/debugging the binary will not work from inside VS.
if(LOVE_MOVE_DLLS)
	foreach(DLL ${LOVE_MOVE_DLLS})
		post_step_move_dll(love ${DLL})
	endforeach()
endif()

if (NOT MSVC OR MINGW)
	return()
endif()

###################################
# Version
###################################

# Extract version.h contents.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/common/version.h LOVE_VERSION_FILE_CONTENTS)

# Extract one of LOVE_VERSION_MAJOR/MINOR/REV.
function(match_version ARG_STRING OUT_VAR)
	string(REGEX MATCH "VERSION_${ARG_STRING} = ([0-9]+);" TMP_VER "${LOVE_VERSION_FILE_CONTENTS}")
	string(REGEX MATCH "[0-9]+" TMP_VER "${TMP_VER}")
	set(${OUT_VAR} ${TMP_VER} PARENT_SCOPE)
endfunction()

match_version("MAJOR" LOVE_VERSION_MAJOR)
match_version("MINOR" LOVE_VERSION_MINOR)
match_version("REV" LOVE_VERSION_REV)

set(LOVE_VERSION_STR "${LOVE_VERSION_MAJOR}.${LOVE_VERSION_MINOR}")

message(STATUS "Version: ${LOVE_VERSION_STR}")

###################################
# CPack
###################################
install(TARGETS ${LOVE_EXE_NAME} ${LOVE_CONSOLE_EXE_NAME} ${LOVE_LIB_NAME} RUNTIME DESTINATION .)

# Extra DLLs.
if(LOVE_EXTRA_DLLS)
	foreach(DLL ${LOVE_EXTRA_DLLS})
		get_filename_component(DLL_NAME ${DLL} NAME)
		message(STATUS "Extra DLL: ${DLL_NAME}")
	endforeach()
	install(FILES ${LOVE_EXTRA_DLLS} DESTINATION .)
endif()

# Dynamic runtime libs.
if(LOVE_MSVC_DLLS)
	foreach(DLL ${LOVE_MSVC_DLLS})
		get_filename_component(DLL_NAME ${DLL} NAME)
		message(STATUS "Runtime DLL: ${DLL_NAME}")
	endforeach()
	install(FILES ${LOVE_MSVC_DLLS} DESTINATION .)
endif()

# Copy a text file from CMAKE_CURRENT_SOURCE_DIR to CMAKE_CURRENT_BINARY_DIR.
# On Windows, this function will convert line endings to CR,LF.
function(copy_text_file ARG_FILE_IN ARG_FILE_OUT)
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_FILE_IN} TMP_TXT_CONTENTS)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_FILE_OUT} ${TMP_TXT_CONTENTS})
endfunction()

# Text files.
copy_text_file(readme.md readme.txt)
copy_text_file(license.txt license.txt)
copy_text_file(changes.txt changes.txt)

install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/changes.txt
		${CMAKE_CURRENT_BINARY_DIR}/license.txt
		${CMAKE_CURRENT_BINARY_DIR}/readme.txt
		DESTINATION .)

# Icons
install(FILES
		${CMAKE_CURRENT_SOURCE_DIR}/extra/nsis/love.ico
		${CMAKE_CURRENT_SOURCE_DIR}/extra/nsis/game.ico
		DESTINATION .)

set(CPACK_GENERATOR ZIP NSIS)
set(CPACK_PACKAGE_NAME "love")
set(CPACK_PACKAGE_VENDOR "love2d.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LOVE -- It's awesome")
set(CPACK_PACKAGE_VERSION "${LOVE_VERSION_STR}")
set(CPACK_PACKAGE_VERSION_MAJOR "${LOVE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${LOVE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${LOVE_VERSION_REV}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "LOVE")
set(CPACK_PACKAGE_EXECUTABLES "${LOVE_EXE_NAME};LOVE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")

set(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
set(CPACK_NSIS_PACKAGE_NAME "LOVE")
set(CPACK_NSIS_DISPLAY_NAME "LOVE ${LOVE_VERSION_STR}")
set(CPACK_NSIS_MODIFY_PATH OFF)

if(LOVE_X64)
	set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
else()
	set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
endif()

set(CPACK_NSIS_MENU_LINKS "http://love2d.org/wiki" "Documentation")

# Some bug somewhere in NSIS requires "\\\\" somewhere in the path,
# according to The Internet. (And sure enough, it does not work
# without it).
set(NSIS_LEFT_BMP "${CMAKE_CURRENT_SOURCE_DIR}/extra/nsis\\\\left.bmp")
set(NSIS_TOP_BMP "${CMAKE_CURRENT_SOURCE_DIR}/extra/nsis\\\\top.bmp")
set(NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/extra/nsis\\\\love.ico")
set(NSIS_MUI_UNICON "${CMAKE_CURRENT_SOURCE_DIR}/extra/nsis\\\\love.ico")

set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "
	!define MUI_WELCOMEPAGE_TITLE \\\"LOVE ${LOVE_VERSION_STR} Setup\\\"
	!define MUI_WELCOMEFINISHPAGE_BITMAP \\\"${NSIS_LEFT_BMP}\\\"
	!define MUI_HEADERIMAGE_BITMAP \\\"${NSIS_TOP_BMP}\\\"
	!define MUI_ICON \\\"${NSIS_MUI_ICON}\\\"
	!define MUI_UNICON \\\"${NSIS_MUI_UNICON}\\\"
")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
	WriteRegStr HKCR \\\".love\\\" \\\"\\\" \\\"LOVE\\\"
	WriteRegStr HKCR \\\"LOVE\\\" \\\"\\\" \\\"LOVE Game File\\\"
	WriteRegStr HKCR \\\"LOVE\\\\DefaultIcon\\\" \\\"\\\" \\\"$INSTDIR\\\\game.ico\\\"
	WriteRegStr HKCR \\\"LOVE\\\\shell\\\" \\\"\\\" \\\"open\\\"
	WriteRegStr HKCR \\\"LOVE\\\\shell\\\\open\\\" \\\"\\\" \\\"Open in LOVE\\\"
	WriteRegStr HKCR \\\"LOVE\\\\shell\\\\open\\\\command\\\" \\\"\\\" \\\"$INSTDIR\\\\love.exe $\\\\\\\"%1$\\\\\\\"\\\"
	System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v  (0x08000000, 0, 0, 0)'
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	DeleteRegKey HKCR \\\"LOVE\\\"
	DeleteRegKey HKCR \\\".love\\\"
	System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v  (0x08000000, 0, 0, 0)'
")

include(CPack)
