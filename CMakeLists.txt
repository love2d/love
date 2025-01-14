#
# Copyright (c) 2006-2024 LOVE Development Team
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

cmake_minimum_required(VERSION 3.16)

project(love)

set(CMAKE_MODULE_PATH "${love_SOURCE_DIR}/extra/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE) # Needed for shared libs on Linux. (-fPIC).
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)	# Allow grouping projects in Visual Studio

if(APPLE)
	message(WARNING "CMake is not an officially supported build system for love on Apple platforms.")
	message(WARNING "Use the prebuilt .app or the xcode project in platform/xcode/ instead.")
endif()

if(MINGW)
	message(WARNING "MinGW is not an officially supported build system for love.")
	message(WARNING "Use megasource with Visual Studio instead.")
	message(WARNING "Please see https://github.com/love2d/megasource")
endif()

include(LoveMacros)

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

set(LOVE_EXE_NAME love CACHE STRING "The name of the executable, usually 'love'")

set(LOVE_DEFAULT_LIB_NAME liblove)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(LOVE_DEFAULT_LIB_NAME "${LOVE_EXE_NAME}-${LOVE_VERSION_STR}")
endif()

set(LOVE_LIB_NAME ${LOVE_DEFAULT_LIB_NAME} CACHE STRING "The name of the lua library, usually 'liblove' or 'love'")
set(LOVE_CONSOLE_EXE_NAME "${LOVE_EXE_NAME}c" CACHE STRING "The name of the console version of the executable, usually 'lovec'")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(LOVE_X64 TRUE)
	set(LOVE_TARGET_PLATFORM x64)
else()
	set(LOVE_X86 TRUE)
	set(LOVE_TARGET_PLATFORM x86)
endif()


if(APPLE)
	set(LOVE_DEFAULT_JIT FALSE)
else()
	set(LOVE_DEFAULT_JIT TRUE)
endif()

option(LOVE_JIT "Use LuaJIT" ${LOVE_DEFAULT_JIT})

if(LOVE_JIT)
	if(APPLE)
		message(WARNING "JIT not supported yet on Mac.")
	endif()
	message(STATUS "LuaJIT: Enabled")
else()
	message(STATUS "LuaJIT: Disabled")
endif()

message(STATUS "Target platform: ${LOVE_TARGET_PLATFORM}")

add_library(lovedep::SDL INTERFACE IMPORTED)
add_library(lovedep::Freetype INTERFACE IMPORTED)
add_library(lovedep::Harfbuzz INTERFACE IMPORTED)
add_library(lovedep::OpenAL INTERFACE IMPORTED)
add_library(lovedep::Modplug INTERFACE IMPORTED)
add_library(lovedep::Theora INTERFACE IMPORTED)
add_library(lovedep::Vorbis INTERFACE IMPORTED)
add_library(lovedep::Ogg INTERFACE IMPORTED)
add_library(lovedep::Zlib INTERFACE IMPORTED)
add_library(lovedep::Lua INTERFACE IMPORTED)

if(MEGA)
	# LOVE_MSVC_DLLS contains runtime DLLs that should be bundled with the love
	# binary (in e.g. the installer). Example: msvcp140.dll.
	set(LOVE_MSVC_DLLS ${MEGA_MSVC_DLLS})

	# These DLLs are moved next to the love binary in a post-build step to
	# love runnable from inside Visual Studio.
	#
	# LOVE_MOVE_DLLS can contain CMake targets, in which case the target's
	# output is assumed to be a DLL, or it can contain paths to actual files.
	# We detect whether or not each item is a target, and take the appropriate
	# action.
	set(LOVE_MOVE_DLLS
		${MEGA_SDL3}
		${MEGA_OPENAL}
	)

	# LOVE_EXTRA_DLLS are non-runtime DLLs which should be bundled with the
	# love binary in installers, etc. It's only needed for external
	# (non-CMake) targets, i.e. LuaJIT.
	if(NOT DEFINED LOVE_EXTRA_DLLS)
		set(LOVE_EXTRA_DLLS)
	endif()

	target_link_libraries(lovedep::SDL INTERFACE ${MEGA_SDL3})
	target_link_libraries(lovedep::Freetype INTERFACE ${MEGA_FREETYPE})
	target_link_libraries(lovedep::Harfbuzz INTERFACE ${MEGA_HARFBUZZ})
	target_link_libraries(lovedep::OpenAL INTERFACE ${MEGA_OPENAL})
	target_link_libraries(lovedep::Modplug INTERFACE ${MEGA_MODPLUG})
	target_link_libraries(lovedep::Theora INTERFACE ${MEGA_LIBTHEORA})
	target_link_libraries(lovedep::Vorbis INTERFACE ${MEGA_LIBVORBIS} ${MEGA_LIBVORBISFILE})
	target_link_libraries(lovedep::Ogg INTERFACE ${MEGA_LIBOGG})
	target_link_libraries(lovedep::Zlib INTERFACE ${MEGA_ZLIB})

	if(LOVE_JIT)
		target_include_directories(lovedep::Lua INTERFACE ${MEGA_LUAJIT_INCLUDE})
		target_link_libraries(lovedep::Lua INTERFACE ${MEGA_LUAJIT_LIB})
		set(LOVE_EXTRA_DLLS ${LOVE_EXTRA_DLLS} ${MEGA_LUAJIT_DLL})
		set(LOVE_EXTRA_DEPENDECIES luajit)
		set(LOVE_MOVE_DLLS
			${LOVE_MOVE_DLLS}
			${MEGA_LUAJIT_DLL}
		)
	else()
		# MEGA_LUA51 is a CMake target, so includes are handled
		# automatically.
		target_link_libraries(lovedep::Lua INTERFACE ${MEGA_LUA51})
		set(LOVE_MOVE_DLLS
			${LOVE_MOVE_DLLS}
			${MEGA_LUA51}
		)
	endif()
else()
	if(MSVC OR ANDROID)
		message(FATAL_ERROR "
It is currently only possible to build with megasource on Windows and Android.
Please see https://github.com/love2d/megasource
")
	endif()

	# required for enet
	add_definitions(-D HAS_SOCKLEN_T)

	find_package(SDL3 3.1.3 REQUIRED CONFIG)
	target_include_directories(lovedep::SDL INTERFACE ${SDL3_INCLUDE_DIRS})
	target_link_libraries(lovedep::SDL INTERFACE ${SDL3_LIBRARIES})

	find_package(Freetype REQUIRED)
	target_include_directories(lovedep::Freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(lovedep::Freetype INTERFACE ${FREETYPE_LIBRARY})

	find_package(Harfbuzz REQUIRED)
	target_include_directories(lovedep::Harfbuzz INTERFACE ${HARFBUZZ_INCLUDE_DIR})
	target_link_libraries(lovedep::Harfbuzz INTERFACE ${HARFBUZZ_LIBRARY})

	find_package(OpenAL REQUIRED)
	target_include_directories(lovedep::OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
	target_link_libraries(lovedep::OpenAL INTERFACE ${OPENAL_LIBRARY})

	find_package(ModPlug REQUIRED)
	target_include_directories(lovedep::Modplug INTERFACE ${MODPLUG_INCLUDE_DIR})
	target_link_libraries(lovedep::Modplug INTERFACE ${MODPLUG_LIBRARY})

	find_package(Theora REQUIRED)
	target_include_directories(lovedep::Theora INTERFACE ${THEORA_INCLUDE_DIR})
	target_link_libraries(lovedep::Theora INTERFACE ${THEORA_LIBRARY} ${THEORADEC_LIBRARY})

	find_package(Vorbis REQUIRED)
	target_include_directories(lovedep::Vorbis INTERFACE ${VORBIS_INCLUDE_DIR})
	target_link_libraries(lovedep::Vorbis INTERFACE ${VORBISFILE_LIBRARY})

	find_package(Ogg REQUIRED)
	target_include_directories(lovedep::Ogg INTERFACE ${OGG_INCLUDE_DIR})
	target_link_libraries(lovedep::Ogg INTERFACE ${OGG_LIBRARY})

	find_package(ZLIB REQUIRED)
	target_include_directories(lovedep::Zlib INTERFACE ${ZLIB_INCLUDE_DIRS})
	target_link_libraries(lovedep::Zlib INTERFACE ${ZLIB_LIBRARY})

	if(LOVE_JIT)
		find_package(LuaJIT REQUIRED)
		target_include_directories(lovedep::Lua INTERFACE ${LUAJIT_INCLUDE_DIR})
		target_link_libraries(lovedep::Lua INTERFACE ${LUAJIT_LIBRARY})
	else()
		find_package(Lua51 REQUIRED)
		target_include_directories(lovedep::Lua INTERFACE ${LUA_INCLUDE_DIR})
		target_link_libraries(lovedep::Lua INTERFACE ${LUA_LIBRARY})
	endif()
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
	target_compile_options(${ARG_TARGET} PRIVATE ${DISABLE_WARNING_FLAG})
endfunction()

#
# common
#

add_library(love_common STATIC
	src/common/android.cpp
	src/common/android.h
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
	src/common/Range.h
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
target_link_libraries(love_common PUBLIC
	lovedep::Lua
	lovedep::SDL
)

if (APPLE)
	target_sources(love_common PRIVATE
		src/common/apple.mm
	)
	target_link_libraries(love_common PUBLIC
		objc
		"-framework CoreFoundation"
	)
	if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		target_sources(love_common PRIVATE
			src/common/macos.mm
		)
		target_link_libraries(love_common PUBLIC
			"-framework AppKit"
		)
	else()
		target_sources(love_common PRIVATE
			src/common/ios.mm
		)
		target_link_libraries(love_common PUBLIC
			"-framework UIKit"
		)
	endif()
endif()

#
# love.audio
#

add_library(love_audio_root STATIC
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
target_link_libraries(love_audio_root PUBLIC
	lovedep::Lua
	lovedep::OpenAL
)

add_library(love_audio_null STATIC
	src/modules/audio/null/Audio.cpp
	src/modules/audio/null/Audio.h
	src/modules/audio/null/Source.cpp
	src/modules/audio/null/Source.h
	src/modules/audio/null/RecordingDevice.cpp
	src/modules/audio/null/RecordingDevice.h
)

add_library(love_audio_openal STATIC
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
target_link_libraries(love_audio_openal PUBLIC
	lovedep::OpenAL
)

add_library(love_audio INTERFACE)
target_link_libraries(love_audio INTERFACE
	love_audio_root
	love_audio_null
	love_audio_openal
)

#
# love.data
#

add_library(love_data STATIC
	src/modules/data/ByteData.cpp
	src/modules/data/ByteData.h
	src/modules/data/CompressedData.cpp
	src/modules/data/CompressedData.h
	src/modules/data/Compressor.cpp
	src/modules/data/Compressor.h
	src/modules/data/DataModule.cpp
	src/modules/data/DataModule.h
	src/modules/data/DataStream.cpp
	src/modules/data/DataStream.h
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
	src/modules/data/wrap_Data.lua
	src/modules/data/wrap_DataModule.cpp
	src/modules/data/wrap_DataModule.h
	src/modules/data/wrap_DataView.cpp
	src/modules/data/wrap_DataView.h
)
target_link_libraries(love_data PUBLIC
	lovedep::Lua
	lovedep::Zlib
)

#
# love.event
#

add_library(love_event_root STATIC
	src/modules/event/Event.cpp
	src/modules/event/Event.h
	src/modules/event/wrap_Event.cpp
	src/modules/event/wrap_Event.h
	src/modules/event/wrap_Event.lua
)
target_link_libraries(love_event_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_event_sdl STATIC
	src/modules/event/sdl/Event.cpp
	src/modules/event/sdl/Event.h
)
target_link_libraries(love_event_sdl PUBLIC
	lovedep::SDL
)

add_library(love_event INTERFACE)
target_link_libraries(love_event INTERFACE
	love_event_root
	love_event_sdl
)

#
# love.filesystem
#

add_library(love_filesystem_root STATIC
	src/modules/filesystem/File.cpp
	src/modules/filesystem/File.h
	src/modules/filesystem/FileData.cpp
	src/modules/filesystem/FileData.h
	src/modules/filesystem/Filesystem.cpp
	src/modules/filesystem/Filesystem.h
	src/modules/filesystem/NativeFile.cpp
	src/modules/filesystem/NativeFile.h
	src/modules/filesystem/wrap_File.cpp
	src/modules/filesystem/wrap_File.h
	src/modules/filesystem/wrap_FileData.cpp
	src/modules/filesystem/wrap_FileData.h
	src/modules/filesystem/wrap_Filesystem.cpp
	src/modules/filesystem/wrap_Filesystem.h
	src/modules/filesystem/wrap_NativeFile.cpp
	src/modules/filesystem/wrap_NativeFile.h
)
target_link_libraries(love_filesystem_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_filesystem_physfs STATIC
	src/modules/filesystem/physfs/File.cpp
	src/modules/filesystem/physfs/File.h
	src/modules/filesystem/physfs/Filesystem.cpp
	src/modules/filesystem/physfs/Filesystem.h
	src/modules/filesystem/physfs/PhysfsIo.h
	src/modules/filesystem/physfs/PhysfsIo.cpp
)
if(ANDROID)
	target_link_libraries(love_filesystem_physfs PUBLIC
		lovedep::SDL
	)
endif()

add_library(love_filesystem INTERFACE)
target_link_libraries(love_filesystem INTERFACE
	love_filesystem_root
	love_filesystem_physfs
)

#
# love.font
#

add_library(love_font_root STATIC
	src/modules/font/BMFontRasterizer.cpp
	src/modules/font/BMFontRasterizer.h
	src/modules/font/Font.cpp
	src/modules/font/Font.h
	src/modules/font/GenericShaper.cpp
	src/modules/font/GenericShaper.h
	src/modules/font/GlyphData.cpp
	src/modules/font/GlyphData.h
	src/modules/font/ImageRasterizer.cpp
	src/modules/font/ImageRasterizer.h
	src/modules/font/Rasterizer.cpp
	src/modules/font/Rasterizer.h
	src/modules/font/TextShaper.cpp
	src/modules/font/TextShaper.h
	src/modules/font/TrueTypeRasterizer.cpp
	src/modules/font/TrueTypeRasterizer.h
	src/modules/font/wrap_Font.cpp
	src/modules/font/wrap_Font.h
	src/modules/font/wrap_GlyphData.cpp
	src/modules/font/wrap_GlyphData.h
	src/modules/font/wrap_Rasterizer.cpp
	src/modules/font/wrap_Rasterizer.h
)
target_link_libraries(love_font_root PUBLIC
	lovedep::Lua
	lovedep::Freetype
)

add_library(love_font_freetype STATIC
	src/modules/font/freetype/Font.cpp
	src/modules/font/freetype/Font.h
	src/modules/font/freetype/HarfbuzzShaper.cpp
	src/modules/font/freetype/HarfbuzzShaper.h
	src/modules/font/freetype/TrueTypeRasterizer.cpp
	src/modules/font/freetype/TrueTypeRasterizer.h
)
target_link_libraries(love_font_freetype PUBLIC
	lovedep::Freetype
	lovedep::Harfbuzz
)

add_library(love_font INTERFACE)
target_link_libraries(love_font INTERFACE
	love_font_root
	love_font_freetype
)

#
# love.graphics
#

add_library(love_graphics_root STATIC
	src/modules/graphics/Buffer.cpp
	src/modules/graphics/Buffer.h
	src/modules/graphics/Deprecations.cpp
	src/modules/graphics/Deprecations.h
	src/modules/graphics/Drawable.cpp
	src/modules/graphics/Drawable.h
	src/modules/graphics/Font.cpp
	src/modules/graphics/Font.h
	src/modules/graphics/Graphics.cpp
	src/modules/graphics/Graphics.h
	src/modules/graphics/GraphicsReadback.cpp
	src/modules/graphics/GraphicsReadback.h
	src/modules/graphics/Mesh.cpp
	src/modules/graphics/Mesh.h
	src/modules/graphics/ParticleSystem.cpp
	src/modules/graphics/ParticleSystem.h
	src/modules/graphics/Polyline.cpp
	src/modules/graphics/Polyline.h
	src/modules/graphics/Quad.cpp
	src/modules/graphics/Quad.h
	src/modules/graphics/renderstate.cpp
	src/modules/graphics/renderstate.h
	src/modules/graphics/Resource.h
	src/modules/graphics/Shader.cpp
	src/modules/graphics/Shader.h
	src/modules/graphics/ShaderStage.cpp
	src/modules/graphics/ShaderStage.h
	src/modules/graphics/SpriteBatch.cpp
	src/modules/graphics/SpriteBatch.h
	src/modules/graphics/StreamBuffer.cpp
	src/modules/graphics/StreamBuffer.h
	src/modules/graphics/TextBatch.cpp
	src/modules/graphics/TextBatch.h
	src/modules/graphics/Texture.cpp
	src/modules/graphics/Texture.h
	src/modules/graphics/vertex.cpp
	src/modules/graphics/vertex.h
	src/modules/graphics/Video.cpp
	src/modules/graphics/Video.h
	src/modules/graphics/Volatile.cpp
	src/modules/graphics/Volatile.h
	src/modules/graphics/wrap_Buffer.cpp
	src/modules/graphics/wrap_Buffer.h
	src/modules/graphics/wrap_Font.cpp
	src/modules/graphics/wrap_Font.h
	src/modules/graphics/wrap_Graphics.cpp
	src/modules/graphics/wrap_Graphics.h
	src/modules/graphics/wrap_Graphics.lua
	src/modules/graphics/wrap_GraphicsReadback.cpp
	src/modules/graphics/wrap_GraphicsReadback.h
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
	src/modules/graphics/wrap_TextBatch.cpp
	src/modules/graphics/wrap_TextBatch.h
	src/modules/graphics/wrap_Video.cpp
	src/modules/graphics/wrap_Video.h
	src/modules/graphics/wrap_Video.lua
)
target_link_libraries(love_graphics_root PUBLIC
	lovedep::Lua
)

add_library(love_graphics_opengl STATIC
	src/modules/graphics/opengl/Buffer.cpp
	src/modules/graphics/opengl/Buffer.h
	src/modules/graphics/opengl/FenceSync.cpp
	src/modules/graphics/opengl/FenceSync.h
	src/modules/graphics/opengl/Graphics.cpp
	src/modules/graphics/opengl/Graphics.h
	src/modules/graphics/opengl/GraphicsReadback.cpp
	src/modules/graphics/opengl/GraphicsReadback.h
	src/modules/graphics/opengl/OpenGL.cpp
	src/modules/graphics/opengl/OpenGL.h
	src/modules/graphics/opengl/Shader.cpp
	src/modules/graphics/opengl/Shader.h
	src/modules/graphics/opengl/ShaderStage.cpp
	src/modules/graphics/opengl/ShaderStage.h
	src/modules/graphics/opengl/StreamBuffer.cpp
	src/modules/graphics/opengl/StreamBuffer.h
	src/modules/graphics/opengl/Texture.cpp
	src/modules/graphics/opengl/Texture.h
)
target_link_libraries(love_graphics_opengl PUBLIC
	lovedep::SDL
)

add_library(love_graphics INTERFACE)
target_link_libraries(love_graphics INTERFACE
	love_graphics_root
	love_graphics_opengl
)

if(APPLE)
	add_library(love_graphics_metal STATIC
		src/modules/graphics/metal/Buffer.h
		src/modules/graphics/metal/Buffer.mm
		src/modules/graphics/metal/Graphics.h
		src/modules/graphics/metal/Graphics.mm
		src/modules/graphics/metal/GraphicsReadback.h
		src/modules/graphics/metal/GraphicsReadback.mm
		src/modules/graphics/metal/Metal.h
		src/modules/graphics/metal/Metal.mm
		src/modules/graphics/metal/Shader.h
		src/modules/graphics/metal/Shader.mm
		src/modules/graphics/metal/ShaderStage.h
		src/modules/graphics/metal/ShaderStage.mm
		src/modules/graphics/metal/StreamBuffer.h
		src/modules/graphics/metal/StreamBuffer.mm
		src/modules/graphics/metal/Texture.h
		src/modules/graphics/metal/Texture.mm
	)
	target_link_libraries(love_graphics_metal PUBLIC
		objc
		"-framework Metal"
		"-framework QuartzCore"
	)
	target_link_libraries(love_graphics INTERFACE
		love_graphics_metal
	)
else()
	add_library(love_graphics_vulkan STATIC
		src/modules/graphics/vulkan/Graphics.h
		src/modules/graphics/vulkan/Graphics.cpp
		src/modules/graphics/vulkan/GraphicsReadback.h
		src/modules/graphics/vulkan/GraphicsReadback.cpp
		src/modules/graphics/vulkan/Shader.h
		src/modules/graphics/vulkan/Shader.cpp
		src/modules/graphics/vulkan/ShaderStage.h
		src/modules/graphics/vulkan/ShaderStage.cpp
		src/modules/graphics/vulkan/StreamBuffer.h
		src/modules/graphics/vulkan/StreamBuffer.cpp
		src/modules/graphics/vulkan/Buffer.h
		src/modules/graphics/vulkan/Buffer.cpp
		src/modules/graphics/vulkan/Texture.h
		src/modules/graphics/vulkan/Texture.cpp
		src/modules/graphics/vulkan/Vulkan.h
		src/modules/graphics/vulkan/Vulkan.cpp
		src/modules/graphics/vulkan/VulkanWrapper.h
	)
	target_link_libraries(love_graphics_vulkan PUBLIC
		lovedep::SDL
	)
	target_link_libraries(love_graphics INTERFACE
		love_graphics_vulkan
	)
endif()

#
# love.image
#

add_library(love_image_root STATIC
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
	src/modules/image/wrap_ImageData.lua
)
target_link_libraries(love_image_root PUBLIC
	lovedep::Lua
)

add_library(love_image_magpie STATIC
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
target_link_libraries(love_image_magpie PUBLIC
	lovedep::Zlib
)

add_library(love_image INTERFACE)
target_link_libraries(love_image INTERFACE
	love_image_root
	love_image_magpie
)

#
# love.joystick
#

add_library(love_joystick_root STATIC
	src/modules/joystick/Joystick.cpp
	src/modules/joystick/Joystick.h
	src/modules/joystick/JoystickModule.h
	src/modules/joystick/wrap_Joystick.cpp
	src/modules/joystick/wrap_Joystick.h
	src/modules/joystick/wrap_JoystickModule.cpp
	src/modules/joystick/wrap_JoystickModule.h
)
target_link_libraries(love_joystick_root PUBLIC
	lovedep::Lua
)

add_library(love_joystick_sdl STATIC
	src/modules/joystick/sdl/Joystick.cpp
	src/modules/joystick/sdl/Joystick.h
	src/modules/joystick/sdl/JoystickModule.cpp
	src/modules/joystick/sdl/JoystickModule.h
)
target_link_libraries(love_joystick_sdl PUBLIC
	lovedep::SDL
)

add_library(love_joystick INTERFACE)
target_link_libraries(love_joystick INTERFACE
	love_joystick_root
	love_joystick_sdl
)

#
# love.keyboard
#

add_library(love_keyboard_root STATIC
	src/modules/keyboard/Keyboard.cpp
	src/modules/keyboard/Keyboard.h
	src/modules/keyboard/wrap_Keyboard.cpp
	src/modules/keyboard/wrap_Keyboard.h
)
target_link_libraries(love_keyboard_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_keyboard_sdl STATIC
	src/modules/keyboard/sdl/Keyboard.cpp
	src/modules/keyboard/sdl/Keyboard.h
)
target_link_libraries(love_keyboard_sdl PUBLIC
	lovedep::SDL
)

add_library(love_keyboard INTERFACE)
target_link_libraries(love_keyboard INTERFACE
	love_keyboard_root
	love_keyboard_sdl
)

#
# love.math
#

add_library(love_math STATIC
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
	src/modules/math/wrap_Math.lua
	src/modules/math/wrap_RandomGenerator.cpp
	src/modules/math/wrap_RandomGenerator.h
	src/modules/math/wrap_RandomGenerator.lua
	src/modules/math/wrap_Transform.cpp
	src/modules/math/wrap_Transform.h
)
target_link_libraries(love_math PUBLIC
	lovedep::Lua
)

#
# love.mouse
#

add_library(love_mouse_root STATIC
	src/modules/mouse/Cursor.cpp
	src/modules/mouse/Cursor.h
	src/modules/mouse/Mouse.h
	src/modules/mouse/wrap_Cursor.cpp
	src/modules/mouse/wrap_Cursor.h
	src/modules/mouse/wrap_Mouse.cpp
	src/modules/mouse/wrap_Mouse.h
)
target_link_libraries(love_mouse_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_mouse_sdl STATIC
	src/modules/mouse/sdl/Cursor.cpp
	src/modules/mouse/sdl/Cursor.h
	src/modules/mouse/sdl/Mouse.cpp
	src/modules/mouse/sdl/Mouse.h
)
target_link_libraries(love_mouse_sdl PUBLIC
	lovedep::SDL
)

add_library(love_mouse INTERFACE)
target_link_libraries(love_mouse INTERFACE
	love_mouse_root
	love_mouse_sdl
)

#
# love.physics
#

add_library(love_physics_root STATIC
	src/modules/physics/Body.cpp
	src/modules/physics/Body.h
	src/modules/physics/Joint.cpp
	src/modules/physics/Joint.h
	src/modules/physics/Shape.cpp
	src/modules/physics/Shape.h
)

add_library(love_physics_box2d STATIC
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
target_link_libraries(love_physics_box2d PUBLIC
	lovedep::Lua
)

add_library(love_physics INTERFACE)
target_link_libraries(love_physics INTERFACE
	love_physics_root
	love_physics_box2d
)

#
# love.sensor
#

add_library(love_sensor_root STATIC
	src/modules/sensor/Sensor.cpp
	src/modules/sensor/Sensor.h
	src/modules/sensor/wrap_Sensor.cpp
	src/modules/sensor/wrap_Sensor.h
)
target_link_libraries(love_sensor_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_sensor_sdl STATIC
	src/modules/sensor/sdl/Sensor.cpp
	src/modules/sensor/sdl/Sensor.h
)
target_link_libraries(love_sensor_sdl PUBLIC
	lovedep::SDL
)

add_library(love_sensor INTERFACE)
target_link_libraries(love_sensor INTERFACE
	love_sensor_root
	love_sensor_sdl
)

#
# love.sound
#

add_library(love_sound_root STATIC
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
	src/modules/sound/wrap_SoundData.lua
)
target_link_libraries(love_sound_root PUBLIC
	lovedep::Lua
)

add_library(love_sound_lullaby STATIC
	src/modules/sound/lullaby/FLACDecoder.cpp
	src/modules/sound/lullaby/FLACDecoder.h
	src/modules/sound/lullaby/ModPlugDecoder.cpp
	src/modules/sound/lullaby/ModPlugDecoder.h
	src/modules/sound/lullaby/MP3Decoder.h
	src/modules/sound/lullaby/MP3Decoder.cpp
	src/modules/sound/lullaby/Sound.cpp
	src/modules/sound/lullaby/Sound.h
	src/modules/sound/lullaby/VorbisDecoder.cpp
	src/modules/sound/lullaby/VorbisDecoder.h
	src/modules/sound/lullaby/WaveDecoder.cpp
	src/modules/sound/lullaby/WaveDecoder.h
)
target_link_libraries(love_sound_lullaby PUBLIC
	lovedep::Modplug
	lovedep::Vorbis
	lovedep::Ogg
)

add_library(love_sound INTERFACE)
target_link_libraries(love_sound INTERFACE
	love_sound_root
	love_sound_lullaby
)

#
# love.system
#

add_library(love_system_root STATIC
	src/modules/system/System.cpp
	src/modules/system/System.h
	src/modules/system/wrap_System.cpp
	src/modules/system/wrap_System.h
)
target_link_libraries(love_system_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_system_sdl STATIC
	src/modules/system/sdl/System.cpp
	src/modules/system/sdl/System.h
)
target_link_libraries(love_system_sdl PUBLIC
	lovedep::SDL
)

add_library(love_system INTERFACE)
target_link_libraries(love_system INTERFACE
	love_system_root
	love_system_sdl
)

#
# love.thread
#

add_library(love_thread_root STATIC
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
target_link_libraries(love_thread_root PUBLIC
	lovedep::Lua
)

add_library(love_thread_sdl STATIC
	src/modules/thread/sdl/Thread.cpp
	src/modules/thread/sdl/Thread.h
	src/modules/thread/sdl/threads.cpp
	src/modules/thread/sdl/threads.h
)
target_link_libraries(love_thread_sdl PUBLIC
	lovedep::SDL
)

add_library(love_thread INTERFACE)
target_link_libraries(love_thread INTERFACE
	love_thread_root
	love_thread_sdl
)

#
# love.timer
#

add_library(love_timer STATIC
	src/modules/timer/Timer.cpp
	src/modules/timer/Timer.h
	src/modules/timer/wrap_Timer.cpp
	src/modules/timer/wrap_Timer.h
)
target_link_libraries(love_timer PUBLIC
	lovedep::Lua
)

#
# love.touch
#

add_library(love_touch_root STATIC
	src/modules/touch/Touch.cpp
	src/modules/touch/Touch.h
	src/modules/touch/wrap_Touch.cpp
	src/modules/touch/wrap_Touch.h
)
target_link_libraries(love_touch_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_touch_sdl STATIC
	src/modules/touch/sdl/Touch.cpp
	src/modules/touch/sdl/Touch.h
)
target_link_libraries(love_touch_sdl PUBLIC
	lovedep::SDL
)

add_library(love_touch INTERFACE)
target_link_libraries(love_touch INTERFACE
	love_touch_root
	love_touch_sdl
)
#
# love.video
#

add_library(love_video_root STATIC
	src/modules/video/Video.h
	src/modules/video/VideoStream.cpp
	src/modules/video/VideoStream.h
	src/modules/video/wrap_Video.cpp
	src/modules/video/wrap_Video.h
	src/modules/video/wrap_VideoStream.cpp
	src/modules/video/wrap_VideoStream.h
)
target_link_libraries(love_video_root PUBLIC
	lovedep::Lua
	lovedep::Theora
	lovedep::Ogg
)

add_library(love_video_theora STATIC
	src/modules/video/theora/Video.cpp
	src/modules/video/theora/Video.h
	src/modules/video/theora/OggDemuxer.cpp
	src/modules/video/theora/OggDemuxer.h
	src/modules/video/theora/TheoraVideoStream.cpp
	src/modules/video/theora/TheoraVideoStream.h
)
target_link_libraries(love_video_theora PUBLIC
	lovedep::Theora
	lovedep::Ogg
)

add_library(love_video INTERFACE)
target_link_libraries(love_video INTERFACE
	love_video_root
	love_video_theora
)

#
# love.window
#

add_library(love_window_root STATIC
	src/modules/window/Window.cpp
	src/modules/window/Window.h
	src/modules/window/wrap_Window.cpp
	src/modules/window/wrap_Window.h
)
target_link_libraries(love_window_root PUBLIC
	lovedep::Lua
	lovedep::SDL
)

add_library(love_window_sdl STATIC
	src/modules/window/sdl/Window.cpp
	src/modules/window/sdl/Window.h
)
target_link_libraries(love_window_sdl PUBLIC
	lovedep::SDL
)

add_library(love_window INTERFACE)
target_link_libraries(love_window INTERFACE
	love_window_root
	love_window_sdl
)

###################################
# Third-party libraries
###################################

#
# Box2D
#

add_library(love_3p_box2d
	src/libraries/box2d/Box2D.h
	src/libraries/box2d/collision/b2_broad_phase.cpp
	src/libraries/box2d/collision/b2_chain_shape.cpp
	src/libraries/box2d/collision/b2_circle_shape.cpp
	src/libraries/box2d/collision/b2_collide_circle.cpp
	src/libraries/box2d/collision/b2_collide_edge.cpp
	src/libraries/box2d/collision/b2_collide_polygon.cpp
	src/libraries/box2d/collision/b2_collision.cpp
	src/libraries/box2d/collision/b2_distance.cpp
	src/libraries/box2d/collision/b2_dynamic_tree.cpp
	src/libraries/box2d/collision/b2_edge_shape.cpp
	src/libraries/box2d/collision/b2_polygon_shape.cpp
	src/libraries/box2d/collision/b2_time_of_impact.cpp
	src/libraries/box2d/common/b2_block_allocator.cpp
	src/libraries/box2d/common/b2_draw.cpp
	src/libraries/box2d/common/b2_math.cpp
	src/libraries/box2d/common/b2_settings.cpp
	src/libraries/box2d/common/b2_stack_allocator.cpp
	src/libraries/box2d/common/b2_timer.cpp
	src/libraries/box2d/dynamics/b2_body.cpp
	src/libraries/box2d/dynamics/b2_chain_circle_contact.cpp
	src/libraries/box2d/dynamics/b2_chain_circle_contact.h
	src/libraries/box2d/dynamics/b2_chain_polygon_contact.cpp
	src/libraries/box2d/dynamics/b2_chain_polygon_contact.h
	src/libraries/box2d/dynamics/b2_circle_contact.cpp
	src/libraries/box2d/dynamics/b2_circle_contact.h
	src/libraries/box2d/dynamics/b2_contact.cpp
	src/libraries/box2d/dynamics/b2_contact_manager.cpp
	src/libraries/box2d/dynamics/b2_contact_solver.cpp
	src/libraries/box2d/dynamics/b2_contact_solver.h
	src/libraries/box2d/dynamics/b2_distance_joint.cpp
	src/libraries/box2d/dynamics/b2_edge_circle_contact.cpp
	src/libraries/box2d/dynamics/b2_edge_circle_contact.h
	src/libraries/box2d/dynamics/b2_edge_polygon_contact.cpp
	src/libraries/box2d/dynamics/b2_edge_polygon_contact.h
	src/libraries/box2d/dynamics/b2_fixture.cpp
	src/libraries/box2d/dynamics/b2_friction_joint.cpp
	src/libraries/box2d/dynamics/b2_gear_joint.cpp
	src/libraries/box2d/dynamics/b2_island.cpp
	src/libraries/box2d/dynamics/b2_island.h
	src/libraries/box2d/dynamics/b2_joint.cpp
	src/libraries/box2d/dynamics/b2_motor_joint.cpp
	src/libraries/box2d/dynamics/b2_mouse_joint.cpp
	src/libraries/box2d/dynamics/b2_polygon_circle_contact.cpp
	src/libraries/box2d/dynamics/b2_polygon_circle_contact.h
	src/libraries/box2d/dynamics/b2_polygon_contact.cpp
	src/libraries/box2d/dynamics/b2_polygon_contact.h
	src/libraries/box2d/dynamics/b2_prismatic_joint.cpp
	src/libraries/box2d/dynamics/b2_pulley_joint.cpp
	src/libraries/box2d/dynamics/b2_revolute_joint.cpp
	src/libraries/box2d/dynamics/b2_weld_joint.cpp
	src/libraries/box2d/dynamics/b2_wheel_joint.cpp
	src/libraries/box2d/dynamics/b2_world.cpp
	src/libraries/box2d/dynamics/b2_world_callbacks.cpp
	src/libraries/box2d/rope/b2_rope.cpp
)
love_disable_warnings(love_3p_box2d)

#
# ddsparse
#

add_library(love_3p_ddsparse
	src/libraries/ddsparse/ddsinfo.h
	src/libraries/ddsparse/ddsparse.cpp
	src/libraries/ddsparse/ddsparse.h
)

#
# dr_flac
#

# dr_flac has no implementation files of its own.
#add_library(love_3p_drflac
#	src/libraries/dr/dr_flac.h
#)

#
# dr_mp3
#

# dr_mp3 has no implementation files of its own.
#add_library(love_3p_drmp3
#	src/libraries/dr/dr_mp3.h
#)

#
# enet
#

add_library(love_3p_enet
	src/libraries/enet/enet.cpp
	src/libraries/enet/lua-enet.h
	src/libraries/enet/libenet/callbacks.c
	src/libraries/enet/libenet/compress.c
	src/libraries/enet/libenet/host.c
	src/libraries/enet/libenet/list.c
	src/libraries/enet/libenet/packet.c
	src/libraries/enet/libenet/peer.c
	src/libraries/enet/libenet/protocol.c
	src/libraries/enet/libenet/unix.c
	src/libraries/enet/libenet/win32.c
	src/libraries/enet/libenet/include/enet/enet.h
	src/libraries/enet/libenet/include/enet/list.h
	src/libraries/enet/libenet/include/enet/protocol.h
	src/libraries/enet/libenet/include/enet/time.h
	src/libraries/enet/libenet/include/enet/types.h
	src/libraries/enet/libenet/include/enet/unix.h
	src/libraries/enet/libenet/include/enet/utility.h
	src/libraries/enet/libenet/include/enet/win32.h
)
love_disable_warnings(love_3p_enet)
target_link_libraries(love_3p_enet lovedep::Lua)
target_include_directories(love_3p_enet PUBLIC src/libraries/enet/libenet/include)
if(MINGW)
	target_link_libraries(love_3p_enet winmm.a)
endif()

#
# GLAD
#

add_library(love_3p_glad
	src/libraries/glad/glad.cpp
	src/libraries/glad/glad.hpp
	src/libraries/glad/gladfuncs.hpp
)

#
# glslang
#

add_library(love_3p_glslang
	src/libraries/glslang/glslang/build_info.h
	src/libraries/glslang/glslang/GenericCodeGen/CodeGen.cpp
	src/libraries/glslang/glslang/GenericCodeGen/Link.cpp
	src/libraries/glslang/glslang/Include/arrays.h
	src/libraries/glslang/glslang/Include/BaseTypes.h
	src/libraries/glslang/glslang/Include/Common.h
	src/libraries/glslang/glslang/Include/ConstantUnion.h
	src/libraries/glslang/glslang/Include/InfoSink.h
	src/libraries/glslang/glslang/Include/InitializeGlobals.h
	src/libraries/glslang/glslang/Include/intermediate.h
	src/libraries/glslang/glslang/Include/PoolAlloc.h
	src/libraries/glslang/glslang/Include/ResourceLimits.h
	src/libraries/glslang/glslang/Include/ShHandle.h
	src/libraries/glslang/glslang/Include/SpirvIntrinsics.h
	src/libraries/glslang/glslang/Include/Types.h
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpContext.h
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpTokens.h
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
	src/libraries/glslang/glslang/MachineIndependent/span.h
	src/libraries/glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp
	src/libraries/glslang/glslang/MachineIndependent/SymbolTable.cpp
	src/libraries/glslang/glslang/MachineIndependent/SymbolTable.h
	src/libraries/glslang/glslang/MachineIndependent/Versions.cpp
	src/libraries/glslang/glslang/MachineIndependent/Versions.h
	src/libraries/glslang/glslang/OSDependent/osinclude.h
	src/libraries/glslang/glslang/Public/ResourceLimits.h
	src/libraries/glslang/glslang/Public/ShaderLang.h
	src/libraries/glslang/glslang/ResourceLimits/ResourceLimits.cpp
	src/libraries/glslang/SPIRV/bitutils.h
	src/libraries/glslang/SPIRV/disassemble.cpp
	src/libraries/glslang/SPIRV/disassemble.h
	src/libraries/glslang/SPIRV/doc.cpp
	src/libraries/glslang/SPIRV/doc.h
	src/libraries/glslang/SPIRV/GLSL.ext.AMD.h
	src/libraries/glslang/SPIRV/GLSL.ext.ARM.h
	src/libraries/glslang/SPIRV/GLSL.ext.EXT.h
	src/libraries/glslang/SPIRV/GLSL.ext.KHR.h
	src/libraries/glslang/SPIRV/GLSL.ext.NV.h
	src/libraries/glslang/SPIRV/GLSL.ext.QCOM.h
	src/libraries/glslang/SPIRV/GLSL.std.450.h
	src/libraries/glslang/SPIRV/GlslangToSpv.cpp
	src/libraries/glslang/SPIRV/GlslangToSpv.h
	src/libraries/glslang/SPIRV/hex_float.h
	src/libraries/glslang/SPIRV/InReadableOrder.cpp
	src/libraries/glslang/SPIRV/Logger.cpp
	src/libraries/glslang/SPIRV/Logger.h
	src/libraries/glslang/SPIRV/NonSemanticDebugPrintf.h
	src/libraries/glslang/SPIRV/NonSemanticShaderDebugInfo100.h
	src/libraries/glslang/SPIRV/spirv.hpp
	src/libraries/glslang/SPIRV/SpvBuilder.cpp
	src/libraries/glslang/SPIRV/SpvBuilder.h
	src/libraries/glslang/SPIRV/spvIR.h
	src/libraries/glslang/SPIRV/SpvPostProcess.cpp
	src/libraries/glslang/SPIRV/SPVRemapper.cpp
	src/libraries/glslang/SPIRV/SPVRemapper.h
	src/libraries/glslang/SPIRV/SpvTools.cpp
	src/libraries/glslang/SPIRV/SpvTools.h
)

if(MSVC OR MINGW)
	target_sources(love_3p_glslang PRIVATE
		src/libraries/glslang/glslang/OSDependent/Windows/ossource.cpp
	)
else()
	target_sources(love_3p_glslang PRIVATE
		src/libraries/glslang/glslang/OSDependent/Unix/ossource.cpp
	)
endif()

#
# LodePNG
#

add_library(love_3p_lodepng
	src/libraries/lodepng/lodepng.cpp
	src/libraries/lodepng/lodepng.h
)

#
# luasocket
#

if(MINGW)
	set(WIN32_LIB_EXT .a)
else()
	set(WIN32_LIB_EXT .lib)
endif()

add_library(love_3p_luasocket
	src/libraries/luasocket/luasocket.cpp
	src/libraries/luasocket/luasocket.h
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
	src/libraries/luasocket/libluasocket/unix.c
	src/libraries/luasocket/libluasocket/unix.h
	src/libraries/luasocket/libluasocket/unixdgram.c
	src/libraries/luasocket/libluasocket/unixdgram.h
	src/libraries/luasocket/libluasocket/unixstream.c
	src/libraries/luasocket/libluasocket/unixstream.h
)
target_link_libraries(love_3p_luasocket lovedep::Lua)
love_disable_warnings(love_3p_luasocket)

if(MSVC OR MINGW)
	target_sources(love_3p_luasocket PRIVATE
		src/libraries/luasocket/libluasocket/wsocket.c
		src/libraries/luasocket/libluasocket/wsocket.h
	)

	target_link_libraries(love_3p_luasocket
		ws2_32${WIN32_LIB_EXT}
	)
else()
	target_sources(love_3p_luasocket PRIVATE
		src/libraries/luasocket/libluasocket/serial.c
		src/libraries/luasocket/libluasocket/usocket.c
		src/libraries/luasocket/libluasocket/usocket.h
	)
endif()

#
# APIs from Lua 5.3
#

add_library(love_3p_lua53
	src/libraries/lua53/lprefix.h
	src/libraries/lua53/lstrlib.c
	src/libraries/lua53/lstrlib.h
	src/libraries/lua53/lutf8lib.c
	src/libraries/lua53/lutf8lib.h
)
target_link_libraries(love_3p_lua53 lovedep::Lua)

#
# Lua HTTPS
#

add_library(love_3p_luahttps
	# These are platform-dependent but have ifdef guards to make sure they only
	# compile on supported platforms.
	src/libraries/luahttps/src/android/AndroidClient.cpp
	src/libraries/luahttps/src/android/AndroidClient.h
	src/libraries/luahttps/src/common/config.h
	src/libraries/luahttps/src/common/Connection.h
	src/libraries/luahttps/src/common/ConnectionClient.h
	src/libraries/luahttps/src/common/HTTPRequest.cpp
	src/libraries/luahttps/src/common/HTTPRequest.h
	src/libraries/luahttps/src/common/HTTPS.cpp
	src/libraries/luahttps/src/common/HTTPS.h
	src/libraries/luahttps/src/common/HTTPSClient.cpp
	src/libraries/luahttps/src/common/HTTPSClient.h
	src/libraries/luahttps/src/common/LibraryLoader.h
	src/libraries/luahttps/src/common/PlaintextConnection.cpp
	src/libraries/luahttps/src/common/PlaintextConnection.h
	src/libraries/luahttps/src/generic/CurlClient.cpp
	src/libraries/luahttps/src/generic/CurlClient.h
	src/libraries/luahttps/src/generic/LinktimeLibraryLoader.cpp
	src/libraries/luahttps/src/generic/OpenSSLConnection.cpp
	src/libraries/luahttps/src/generic/OpenSSLConnection.h
	src/libraries/luahttps/src/generic/UnixLibraryLoader.cpp
	src/libraries/luahttps/src/lua/main.cpp
	src/libraries/luahttps/src/windows/SChannelConnection.cpp
	src/libraries/luahttps/src/windows/SChannelConnection.h
	src/libraries/luahttps/src/windows/WindowsLibraryLoader.cpp
	src/libraries/luahttps/src/windows/WinINetClient.cpp
	src/libraries/luahttps/src/windows/WinINetClient.h
)
target_link_libraries(love_3p_luahttps lovedep::Lua)

if (APPLE)
	target_sources(love_3p_luahttps PRIVATE
		src/libraries/luahttps/src/apple/NSURLClient.mm
		src/libraries/luahttps/src/apple/NSURLClient.h
	)
	target_compile_options(love_3p_luahttps PRIVATE -fobjc-arc)
endif()

if(MSVC)
	target_link_libraries(love_3p_luahttps
		ws2_32
		secur32
	)

	if(NOT CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
		target_link_libraries(love_3p_luahttps
			wininet
		)
	endif()
endif()

#
# lz4
#

add_library(love_3p_lz4
	src/libraries/lz4/lz4.c
	src/libraries/lz4/lz4.h
	src/libraries/lz4/lz4hc.c
	src/libraries/lz4/lz4hc.h
	src/libraries/lz4/lz4opt.h
)

#
# noise1234
#

add_library(love_3p_noise1234
	src/libraries/noise1234/noise1234.cpp
	src/libraries/noise1234/noise1234.h
	src/libraries/noise1234/simplexnoise1234.cpp
	src/libraries/noise1234/simplexnoise1234.h
)

#
# physfs
#

add_library(love_3p_physfs
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
	src/libraries/physfs/physfs_platform_android.c
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
love_disable_warnings(love_3p_physfs)

if(APPLE)
	target_sources(love_3p_physfs PRIVATE
		src/libraries/physfs/physfs_platform_apple.m
	)
	target_link_libraries(love_3p_physfs INTERFACE
		"-framework IOKit"
	)
endif()

#
# spirv_cross
#

add_library(love_3p_spirv_cross
	src/libraries/spirv_cross/GLSL.std.450.h
	src/libraries/spirv_cross/spirv_cfg.cpp
	src/libraries/spirv_cross/spirv_cfg.hpp
	src/libraries/spirv_cross/spirv_common.hpp
	src/libraries/spirv_cross/spirv_cpp.cpp
	src/libraries/spirv_cross/spirv_cpp.hpp
	src/libraries/spirv_cross/spirv_cross_containers.hpp
	src/libraries/spirv_cross/spirv_cross_error_handling.hpp
	src/libraries/spirv_cross/spirv_cross_parsed_ir.cpp
	src/libraries/spirv_cross/spirv_cross_parsed_ir.hpp
	src/libraries/spirv_cross/spirv_cross_util.cpp
	src/libraries/spirv_cross/spirv_cross_util.hpp
	src/libraries/spirv_cross/spirv_cross.cpp
	src/libraries/spirv_cross/spirv_cross.hpp
	src/libraries/spirv_cross/spirv_glsl.cpp
	src/libraries/spirv_cross/spirv_glsl.hpp
	src/libraries/spirv_cross/spirv_hlsl.cpp
	src/libraries/spirv_cross/spirv_hlsl.hpp
	src/libraries/spirv_cross/spirv_msl.cpp
	src/libraries/spirv_cross/spirv_msl.hpp
	src/libraries/spirv_cross/spirv_parser.cpp
	src/libraries/spirv_cross/spirv_parser.hpp
	src/libraries/spirv_cross/spirv_reflect.cpp
	src/libraries/spirv_cross/spirv_reflect.hpp
	src/libraries/spirv_cross/spirv.hpp
)

#
# stb_image
#

# stb_image has no implementation files of its own.
#add_library(love_3p_stb
#	src/libraries/stb/stb_image.h
#)

#
# tiny exr
#

# tinyexr has no implementation files of its own.
#add_library(love_3p_tinyexr
#	src/libraries/tinyexr/tinyexr.h
#)

#
# utf8
#

# This library is all headers ... so there is no need to
# add_library() here.
#add_library(love_3p_utf8
#	src/libraries/utf8/utf8.h
#	src/libraries/utf8/utf8/checked.h
#	src/libraries/utf8/utf8/core.h
#	src/libraries/utf8/utf8/unchecked.h
#)

#
# vma
#

# vulkan memory allocatory has no implementation files of its own.
#add_library(love_3p_vma
#	src/libraries/vma/vk_mem_alloc.h
#)

#
# volk
#

# since we don't want to use the system vulkan header files we need to 
# compile this library in the löve source code using VOLK_IMPLEMENTATION.
#add_library(love_3p_volk 
#	src/libraries/volk/volk.h
#	src/libraries/volk/volk.c)

#
# vulkan headers
#

# vulkan headers has no implementation files of its own.
#add_library(love_3p_vulkan_headers
#	src/libraries/vulkanheaders/vk_icd.h
#	src/libraries/vulkanheaders/vk_layer.h
#	src/libraries/vulkanheaders/vk_platform.h
#	src/libraries/vulkanheaders/vk_sdk-platform.h
#	src/libraries/vulkanheaders/vulkan_android.h
#	src/libraries/vulkanheaders/vulkan_beta.h
#	src/libraries/vulkanheaders/vulkan_core.h
#	src/libraries/vulkanheaders/vulkan_directfb.h
#	src/libraries/vulkanheaders/vulkan_enums.hpp
#	src/libraries/vulkanheaders/vulkan_format_traits.hpp
#	src/libraries/vulkanheaders/vulkan_fuchsia.h
#	src/libraries/vulkanheaders/vulkan_funcs.h
#	src/libraries/vulkanheaders/vulkan_ggp.h
#	src/libraries/vulkanheaders/vulkan_handles.h
#	src/libraries/vulkanheaders/vulkan_hash.hpp
#	src/libraries/vulkanheaders/vulkan_ios.h
#	src/libraries/vulkanheaders/vulkan_macos.h
#	src/libraries/vulkanheaders/vulkan_metal.h
#	src/libraries/vulkanheaders/vulkan_raii.hpp
#	src/libraries/vulkanheaders/vulkan_screen.h
#	src/libraries/vulkanheaders/vulkan_static_assertions.h
#	src/libraries/vulkanheaders/vulkan_structs.hpp
#	src/libraries/vulkanheaders/vulkan_to_string.h
#	src/libraries/vulkanheaders/vulkan_vi.h
#	src/libraries/vulkanheaders/vulkan_wayland.h
#	src/libraries/vulkanheaders/vulkan_win32.h
#	src/libraries/vulkanheaders/vulkan_xcb.h
#	src/libraries/vulkanheaders/vulkan_xlib_xrandr.h
#	src/libraries/vulkanheaders/vulkan_xlib.h
#	src/libraries/vulkanheaders/vulkan.h
#	src/libraries/vulkanheaders/vulkan.hpp
#)

#
# Wuff
#

add_library(love_3p_wuff
	src/libraries/Wuff/wuff.c
	src/libraries/Wuff/wuff.h
	src/libraries/Wuff/wuff_config.h
	src/libraries/Wuff/wuff_convert.c
	src/libraries/Wuff/wuff_convert.h
	src/libraries/Wuff/wuff_internal.c
	src/libraries/Wuff/wuff_internal.h
	src/libraries/Wuff/wuff_memory.c
)

#
# xxHash
#

add_library(love_3p_xxhash
	src/libraries/xxHash/xxhash.c
	src/libraries/xxHash/xxhash.h
)

#
# liblove
#
include_directories(
	BEFORE
	src
	src/libraries
	src/libraries/box2D
	src/modules
)

if(MINGW)
	# UTF-16 flags passed to windres. windres invokes gcc as preprocessor
	# -> gcc outputs utf8, so windres must read-in codepage 65001 (utf8)
	set(CMAKE_RC_FLAGS ${CMAKE_RC_FLAGS} "-c 65001 --preprocessor-arg=-finput-charset=UTF-16LE")
endif()

add_library(liblove SHARED
	src/modules/love/love.cpp
	src/modules/love/love.h
	# These are here so they show up in Visual Studio:
	src/modules/love/arg.lua
	src/modules/love/boot.lua
	src/modules/love/callbacks.lua
	src/modules/love/jitsetup.lua
	src/scripts/nogame.lua
)
set_target_properties(liblove PROPERTIES
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	LIBRARY_OUTPUT_NAME "${LOVE_LIB_NAME}")
set(LIBLOVE_DEPENDENCIES
	love_common
	love_audio
	love_data
	love_event
	love_filesystem
	love_font
	love_graphics
	love_image
	love_joystick
	love_keyboard
	love_math
	love_mouse
	love_physics
	love_sensor
	love_sound
	love_system
	love_thread
	love_timer
	love_touch
	love_video
	love_window
)
set(LIBLOVE_LIBRARIES
	love_3p_box2d
	love_3p_ddsparse
	love_3p_enet
	love_3p_glad
	love_3p_glslang
	love_3p_lodepng
	love_3p_luasocket
	love_3p_lua53
	love_3p_luahttps
	love_3p_lz4
	love_3p_noise1234
	love_3p_physfs
	love_3p_spirv_cross
	love_3p_wuff
	love_3p_xxhash
)
target_link_libraries(liblove ${LIBLOVE_DEPENDENCIES} ${LIBLOVE_LIBRARIES})

if(LOVE_EXTRA_DEPENDECIES)
	add_dependencies(liblove ${LOVE_EXTRA_DEPENDECIES})
endif()

#
# Group projects in Visual Studio
#

love_group_projects(NAME "liblove" NESTED TARGETS ${LIBLOVE_DEPENDENCIES})
love_group_projects(NAME "liblove/libraries" NESTED TARGETS ${LIBLOVE_LIBRARIES})
love_group_projects(NAME "liblove" TARGETS liblove ${LOVE_EXTRA_DEPENDECIES})

love_group_projects(NAME "lovedep" TARGETS lovedep::SDL lovedep::Freetype lovedep::Harfbuzz lovedep::OpenAL lovedep::Modplug lovedep::Theora lovedep::Vorbis lovedep::Ogg lovedep::Zlib lovedep::Lua)
love_group_projects(NAME "lovedep" TARGETS lua51 alcommon al-excommon harfbuzz-subset zlib)


if(MSVC)
	set_target_properties(liblove PROPERTIES RELEASE_OUTPUT_NAME "love" PDB_NAME "liblove" IMPORT_PREFIX "lib")
	set_target_properties(liblove PROPERTIES DEBUG_OUTPUT_NAME "love" PDB_NAME "liblove" IMPORT_PREFIX "lib")
endif()

if(MSVC OR MINGW)
	target_link_libraries(liblove
		ws2_32${WIN32_LIB_EXT}
		winmm${WIN32_LIB_EXT}
		dwmapi${WIN32_LIB_EXT}
	)

	target_sources(liblove PUBLIC
		extra/windows/love.rc
		extra/windows/love.ico
	)
endif()

#
# love (executable)
#
if(ANDROID)
	add_library(love SHARED) # On Android, the LOVE main entrypoint needs to be compiled as shared library
	target_link_libraries(liblove android)
else()
	add_executable(love WIN32)
endif()

target_sources(love PRIVATE src/love.cpp)
target_link_libraries(love liblove)
set_target_properties(love PROPERTIES
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	OUTPUT_NAME ${LOVE_EXE_NAME})

if(MSVC OR MINGW)
	add_executable(lovec src/love.cpp)
	target_link_libraries(lovec liblove)
	set_target_properties(lovec PROPERTIES
		OUTPUT_NAME ${LOVE_CONSOLE_EXE_NAME})
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

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	###################################
	# CPack
	###################################
	install(TARGETS love lovec liblove RUNTIME DESTINATION .)
	install(PROGRAMS $<TARGET_FILE:${MEGA_SDL3}> DESTINATION .)

	# Our install script (and NSIS) doesn't fully support Windows ARM64 yet.
	if(MEGA_ARM64)
		set(CPACK_GENERATOR ZIP)
		set(CPACK_SYSTEM_NAME woa64)
	else()
		set(CPACK_GENERATOR ZIP NSIS)
	endif()

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
	set(CPACK_NSIS_MODIFY_PATH ON)

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
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	include(GNUInstallDirs)

	string(TIMESTAMP LOVE_CONFIGURE_TIMESTAMP "%a, %d %b %Y %H:%M:%S +0000" UTC)

	file(COPY platform/unix/debian/ DESTINATION debian PATTERN "*.in" EXCLUDE)
	configure_file(platform/unix/debian/control.in debian/control @ONLY)
	configure_file(platform/unix/debian/changelog.in debian/changelog @ONLY)
	configure_file(platform/unix/debian/rules.in debian/rules @ONLY)

	configure_file(platform/unix/love.desktop.in love.desktop @ONLY)

	install(TARGETS love liblove)
	install(FILES platform/unix/love.6
			DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
			RENAME love.1)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/love.desktop
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
	install(FILES platform/unix/love.xml
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/mime/packages)
	install(FILES platform/unix/love.svg
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps)
	install(FILES platform/unix/application-x-love-game.svg
			DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/mimetypes)
endif()
