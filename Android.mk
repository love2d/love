LOCAL_PATH:= $(call my-dir)

# libogg
include $(CLEAR_VARS)

LOCAL_MODULE    := liblove
LOCAL_CFLAGS    := -fexceptions -g -Dlinux -Dunix \
	-DHAVE_GCC_DESTRUCTOR=1 -DOPT_GENERIC -DREAL_IS_FLOAT \
	-DGL_GLEXT_PROTOTYPES -DAL_ALEXT_PROTOTYPES

LOCAL_CPPFLAGS  := ${LOCAL_CFLAGS} 

# I don't think there's armeabi-v7a device without NEON instructions in 2018
LOCAL_ARM_NEON := true

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	# ARM64 does have socklen_t
	LOCAL_CFLAGS += -DHAS_SOCKLEN_T=1
endif

LOCAL_C_INCLUDES  :=  \
	${LOCAL_PATH}/src \
	${LOCAL_PATH}/src/modules \
	${LOCAL_PATH}/src/libraries/ \
	${LOCAL_PATH}/src/libraries/enet/libenet/include \
	${LOCAL_PATH}/src/libraries/physfs \
	${LOCAL_PATH}/src/libraries/glslang/glslang/Include \
	${LOCAL_PATH}/../SDL2-2.0.9/include \
	${LOCAL_PATH}/../openal-soft-1.18.2/include \
	${LOCAL_PATH}/../openal-soft-1.18.2/OpenAL32/Include \
	${LOCAL_PATH}/../freetype2-android/include \
	${LOCAL_PATH}/../freetype2-android/src \
	${LOCAL_PATH}/../mpg123-1.17.0/src/libmpg123 \
	${LOCAL_PATH}/../libmodplug-0.8.8.4/src \
	${LOCAL_PATH}/../libvorbis-1.3.5/include \
	${LOCAL_PATH}/../LuaJIT-2.1/src \
	${LOCAL_PATH}/../libogg-1.3.2/include \
	${LOCAL_PATH}/../libtheora-1.2.0alpha1/include 
		
LOCAL_SRC_FILES := \
	$(filter-out \
	  src/libraries/luasocket/libluasocket/wsocket.c \
	,$(subst $(LOCAL_PATH)/,,\
	$(wildcard ${LOCAL_PATH}/src/love.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/common/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/audio/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/audio/null/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/audio/openal/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/data/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/event/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/event/sdl/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/filesystem/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/filesystem/physfs/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/font/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/font/freetype/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/graphics/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/graphics/opengl/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/image/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/image/magpie/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/joystick/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/joystick/sdl/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/keyboard/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/keyboard/sdl/*.cpp) \
	$(wildcard ${LOCAL_PATH}/src/modules/love/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/math/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/mouse/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/mouse/sdl/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/physics/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/physics/box2d/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/sound/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/sound/lullaby/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/system/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/system/sdl/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/thread/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/thread/sdl/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/touch/*.cpp) \
 	$(wildcard ${LOCAL_PATH}/src/modules/touch/sdl/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/modules/timer/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/modules/timer/sdl/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/modules/video/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/modules/video/theora/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/modules/window/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/modules/window/sdl/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/ddsparse/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Collision/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Collision/Shapes/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Common/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Dynamics/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Dynamics/Contacts/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Dynamics/Joints/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/Box2D/Rope/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glad/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glslang/glslang/GenericCodeGen/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glslang/glslang/MachineIndependent/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glslang/glslang/MachineIndependent/preprocessor/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glslang/glslang/OSDependent/Unix/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glslang/OGLCompilersDLL/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/glslang/glslang//*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/enet/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/enet/libenet/*.c) \
  $(wildcard ${LOCAL_PATH}/src/libraries/lua53/*.c) \
  $(wildcard ${LOCAL_PATH}/src/libraries/luasocket/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/luautf8/*.c) \
  $(wildcard ${LOCAL_PATH}/src/libraries/luasocket/libluasocket/*.c) \
  $(wildcard ${LOCAL_PATH}/src/libraries/lodepng/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/lz4/*.c) \
  $(wildcard ${LOCAL_PATH}/src/libraries/noise1234/*.cpp) \
  $(wildcard ${LOCAL_PATH}/src/libraries/physfs/*.c) \
	$(wildcard ${LOCAL_PATH}/src/libraries/Wuff/*.c) \
  $(wildcard ${LOCAL_PATH}/src/libraries/xxHash/*.c) \
  ))

LOCAL_CXXFLAGS := -std=c++0x

LOCAL_SHARED_LIBRARIES := libopenal libmpg123 

LOCAL_STATIC_LIBRARIES := libvorbis libogg libtheora libmodplug libfreetype libluajit SDL2_static

# $(info liblove: include dirs $(LOCAL_C_INCLUDES))
# $(info liblove: src files $(LOCAL_SRC_FILES))

SDL_PATH := ../SDL2-2.0.9
LOCAL_SRC_FILES += $(SDL_PATH)/src/main/android/SDL_android_main.c 
LOCAL_LDLIBS := -lz -lGLESv1_CM -lGLESv2 -ldl -landroid
LOCAL_LDFLAGS := -Wl,--allow-multiple-definition

include $(BUILD_SHARED_LIBRARY)
