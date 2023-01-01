LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE    := liblove

LOCAL_CFLAGS    := -g -DGL_GLEXT_PROTOTYPES -DAL_ALEXT_PROTOTYPES -fvisibility=hidden
LOCAL_CPPFLAGS  := -fvisibility-inlines-hidden

# I don't think there's armeabi-v7a device without NEON instructions in 2018
LOCAL_ARM_NEON := true

ifeq ($(IS_ANDROID_21),yes)
	# API21 defines socklen_t
	LOCAL_CFLAGS += -DHAS_SOCKLEN_T=1
endif

LOCAL_C_INCLUDES  :=  \
	${LOCAL_PATH}/src \
	${LOCAL_PATH}/src/modules \
	${LOCAL_PATH}/src/libraries/ \
	${LOCAL_PATH}/src/libraries/enet/libenet/include \
	${LOCAL_PATH}/src/libraries/physfs \
	${LOCAL_PATH}/src/libraries/glslang/glslang/Include

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

LOCAL_CXXFLAGS := -std=c++11
LOCAL_SHARED_LIBRARIES := libopenal libmpg123 
LOCAL_STATIC_LIBRARIES := libvorbis libogg libtheora libmodplug libfreetype libluajit SDL2_static

# $(info liblove: include dirs $(LOCAL_C_INCLUDES))
# $(info liblove: src files $(LOCAL_SRC_FILES))

LOCAL_LDLIBS := -lz -lGLESv1_CM -lGLESv2 -ldl -landroid
LOCAL_LDFLAGS := -Wl,--allow-multiple-definition

include $(BUILD_SHARED_LIBRARY)
