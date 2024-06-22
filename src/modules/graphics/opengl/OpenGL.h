/**
 * Copyright (c) 2006-2024 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_GRAPHICS_OPENGL_OPENGL_H
#define LOVE_GRAPHICS_OPENGL_OPENGL_H

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "common/math.h"
#include "common/Color.h"
#include "graphics/Texture.h"
#include "graphics/vertex.h"
#include "graphics/renderstate.h"
#include "common/Matrix.h"

// GLAD
#include "libraries/glad/gladfuncs.hpp"

// C++
#include <vector>
#include <stack>

// The last argument to AttribPointer takes a buffer offset casted to a pointer.
#define BUFFER_OFFSET(i) ((char *) NULL + (i))

namespace love
{
namespace graphics
{

class Resource;
class Buffer;

namespace opengl
{

// Awful, but the library uses the namespace in order to use the functions sanely
// with proper autocomplete in IDEs while having name mangling safety -
// no clashes with other GL libraries when linking, etc.
using namespace glad;

/**
 * Thin layer between OpenGL and the rest of the program.
 * Internally shadows some OpenGL context state for improved efficiency and
 * accuracy (compared to glGet etc.)
 * A class is more convenient and readable than plain namespaced functions, but
 * typically only one OpenGL object should be used (singleton.)
 **/
class OpenGL
{
public:

	// OpenGL GPU vendors.
	enum Vendor
	{
		VENDOR_AMD,
		VENDOR_NVIDIA,
		VENDOR_INTEL,
		VENDOR_MESA_SOFT, // Software renderer.
		VENDOR_APPLE,     // Software renderer on desktops.
		VENDOR_MICROSOFT, // Software renderer.
		VENDOR_IMGTEC,
		VENDOR_ARM,
		VENDOR_QUALCOMM,
		VENDOR_BROADCOM,
		VENDOR_VIVANTE,
		VENDOR_UNKNOWN
	};

	enum FramebufferTarget
	{
		FRAMEBUFFER_READ = (1 << 0),
		FRAMEBUFFER_DRAW = (1 << 1),
		FRAMEBUFFER_ALL  = (FRAMEBUFFER_READ | FRAMEBUFFER_DRAW),
	};

	enum EnableState
	{
		ENABLE_BLEND,
		ENABLE_DEPTH_TEST,
		ENABLE_STENCIL_TEST,
		ENABLE_SCISSOR_TEST,
		ENABLE_FACE_CULL,
		ENABLE_FRAMEBUFFER_SRGB,
		ENABLE_MAX_ENUM
	};

	struct TextureFormat
	{
		GLenum internalformat = 0;
		GLenum externalformat = 0;
		GLenum type = 0;

		// For depth/stencil formats.
		GLenum framebufferAttachments[2];

		bool swizzled = false;
		GLint swizzle[4];
	};

	class TempDebugGroup
	{
	public:

		TempDebugGroup(const char *name);
		~TempDebugGroup();
	};

	// glClear() is affected by various OpenGL state...
	class CleanClearState
	{
	public:
		CleanClearState(GLbitfield clearFlags);
		~CleanClearState();

	private:
		GLenum clearFlags;
		uint32 colorWriteMask;
		uint32 stencilWriteMask;
		bool depthWrites;
		bool scissor;
	};

	struct Stats
	{
		int shaderSwitches;
	} stats;

	struct Bugs
	{
		/**
		 * On AMD's Windows (and probably Linux) drivers,
		 * glBindFramebuffer + glClear + glBindFramebuffer + draw(fbo_tex) won't
		 * work unless there's some kind of draw or state change which causes
		 * the driver to update the texture's contents (just drawing the texture
		 * won't always do it, with this driver bug).
		 * Activating shader program 0 and then activating the actual program
		 * seems to always 'fix' it for me.
		 * Bug observed January 2016 with multiple AMD GPUs and driver versions.
		 * https://love2d.org/forums/viewtopic.php?f=4&t=81496
		 **/
		bool clearRequiresDriverTextureStateUpdate;

		/**
		 * AMD's Windows drivers don't always properly generate mipmaps unless
		 * glEnable(GL_TEXTURE_2D) is called directly before glGenerateMipmap.
		 * This only applies to legacy and Compatibility Profile contexts, of
		 * course.
		 * https://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
		 **/
		bool generateMipmapsRequiresTexture2DEnable;

		/**
		 * Report: Intel HD 4000 on Windows hangs during glClientWaitSync.
		 * I found this when googling the issue:
		 * https://github.com/mjn33/planetgen/commit/235e23873a22e219fffdd9ede706c1051aa0f107
		 **/
		bool clientWaitSyncStalls;

		/**
		 * glTexStorage on some older AMD/ATI graphics drivers on Windows seems
		 * to break subsequent sub-rectangle glTexSubImage calls after an
		 * initial full-size one (determined after some investigation with an
		 * affected user on Discord.)
		 * https://bitbucket.org/rude/love/issues/1436/bug-with-lovegraphicsprint-on-older-ati
		 * https://github.com/love2d/love/issues/1563
		 **/
		bool texStorageBreaksSubImage;

		/**
		 * An Android device with an Adreno 630 (supposedly GLES3.2-capable)
		 * fails with GL_INVALID_OPERATION in glTexImage2D if the image is
		 * GL_R8, despite the GLES 3.0 spec mandating support for that format.
		 * It's possible more Adreno GPUs / drivers are affected as well.
		 **/
		bool brokenR8PixelFormat;

		/**
		 * Intel HD Graphics drivers on Windows prior to the HD 2500/4000 have
		 * completely broken sRGB support.
		 * https://github.com/love2d/love/issues/1592
		 **/
		bool brokenSRGB;

		/**
		 * Other bugs which have workarounds that don't use conditional code at
		 * the moment:
		 *
		 * Kepler nvidia GPUs in at least OS X 10.10 and 10.11 fail to render
		 * geometry with glDrawElements if index data comes from a Buffer Object
		 * and vertex data doesn't. One workaround is to use a CPU-side index
		 * array when there's also a CPU-side vertex array.
		 * https://love2d.org/forums/viewtopic.php?f=4&t=81401&start=10
		 *
		 * Some android drivers don't seem to initialize the sampler index
		 * values of sampler uniforms in shaders to 0 (which is required by the
		 * GLSL ES specification) when linking the shader program. One
		 * workaround is to always set the values of said sampler uniforms to 0
		 * just after linking the shader program.
		 * https://love2d.org/forums/viewtopic.php?f=4&t=81458
		 **/
	} bugs;

	OpenGL();

	/**
	 * Initializes the active OpenGL context.
	 **/
	bool initContext();

	/**
	 * Sets up some required context state based on current and default OpenGL
	 * state. Call this directly after initializing an OpenGL context!
	 **/
	void setupContext();

	/**
	 * Marks current context state as invalid and deletes OpenGL objects owned
	 * by this class instance. Call this directly before potentially deleting
	 * an OpenGL context!
	 **/
	void deInitContext();

	/**
	 * Set up necessary state (LOVE-provided shader uniforms, etc.) for drawing.
	 * This *MUST* be called directly before OpenGL drawing functions.
	 **/
	void prepareDraw(love::graphics::Graphics *gfx);

	/**
	 * State-tracked glBindBuffer.
	 * NOTE: This does not account for multiple VAOs being used! Index buffer
	 * bindings are per-VAO in OpenGL, but this doesn't know about that.
	 **/
	void bindBuffer(BufferUsage type, GLuint buffer);

	/**
	 * glDeleteBuffers which updates our shadowed state.
	 **/
	void deleteBuffer(GLuint buffer);

	/**
	 * Set all vertex attribute state.
	 **/
	void setVertexAttributes(const VertexAttributes &attributes, const BufferBindings &buffers);

	/**
	 * Wrapper for glCullFace which eliminates redundant state setting.
	 **/
	void setCullMode(CullMode mode);

	/**
	 * Wrapper for glClearDepth and glClearDepthf.
	 **/
	void clearDepth(double value);

	/**
	 * Sets the OpenGL rendering viewport to the specified rectangle.
	 * The y-coordinate starts at the top.
	 **/
	void setViewport(const Rect &v);
	Rect getViewport() const;

	/**
	 * Sets the scissor box to the specified rectangle.
	 * The y-coordinate starts at the top and is flipped internally.
	 **/
	void setScissor(const Rect &v, bool rtActive);

	/**
	 * State-tracked version of glEnable.
	 **/
	void setEnableState(EnableState state, bool enable);
	bool isStateEnabled(EnableState state) const;

	/**
	 * Binds a Framebuffer Object to the specified target.
	 **/
	void bindFramebuffer(FramebufferTarget target, GLuint framebuffer);
	GLuint getFramebuffer(FramebufferTarget target) const;
	void deleteFramebuffer(GLuint framebuffer);

	void framebufferTexture(GLenum attachment, TextureType texType, GLuint texture, int level, int layer = 0, int face = 0);

	/**
	 * Calls glDepthMask.
	 **/
	void setDepthWrites(bool enable);
	bool hasDepthWrites() const;

	void setStencilWriteMask(uint32 mask);
	uint32 getStencilWriteMask() const;

	void setColorWriteMask(uint32 mask);
	uint32 getColorWriteMask() const;

	/**
	 * Calls glUseProgram.
	 **/
	void useProgram(GLuint program);

	/**
	 * This will usually be 0 (system drawable), but some platforms require a
	 * non-zero FBO for rendering.
	 **/
	GLuint getDefaultFBO() const;

	/**
	 * Helper for setting the active texture unit.
	 *
	 * @param textureunit Index in the range of [0, maxtextureunits-1]
	 **/
	void setTextureUnit(int textureunit);

	/**
	 * Helper for binding a texture to a specific texture unit.
	 *
	 * @param textureunit Index in the range of [0, maxtextureunits-1]
	 * @param restoreprev Restore previously bound texture unit when done.
	 * @param bindforedit If false, the active texture unit may be left alone.
	 **/
	void bindTextureToUnit(TextureType target, GLuint texture, int textureunit, bool restoreprev, bool bindforedit = true);
	void bindTextureToUnit(Texture *texture, int textureunit, bool restoreprev, bool bindforedit = true);

	void bindBufferTextureToUnit(GLuint texture, int textureunit, bool restoreprev, bool bindforedit);

	void bindIndexedBuffer(GLuint buffer, BufferUsage type, int index);

	/**
	 * Helper for deleting an OpenGL texture.
	 * Cleans up if the texture is currently bound.
	 **/
	void deleteTexture(GLuint texture);

	/**
	 * Sets sampler state parameters for the currently bound texture.
	 **/
	void setSamplerState(TextureType target, SamplerState &s);

	/**
	 * Equivalent to glTexStorage2D/3D on platforms that support it. Equivalent
	 * to glTexImage2D/3D for all levels and slices of a texture otherwise.
	 * NOTE: this does not handle compressed texture formats.
	 **/
	bool rawTexStorage(TextureType target, int levels, PixelFormat pixelformat, int width, int height, int depth = 1);

	bool isBufferUsageSupported(BufferUsage usage) const;
	bool isClampZeroOneTextureWrapSupported() const;
	bool isSamplerLODBiasSupported() const;
	bool isBaseVertexSupported() const;
	bool isCopyTextureToBufferSupported() const;

	/**
	 * Returns the maximum supported width or height of a texture.
	 **/
	int getMax2DTextureSize() const;
	int getMax3DTextureSize() const;
	int getMaxCubeTextureSize() const;
	int getMaxTextureLayers() const;

	/**
	 * Returns the maximum number of values in a texel buffer.
	 **/
	int getMaxTexelBufferSize() const;

	/**
	 * Returns the maximum number of bytes in a shader storage buffer.
	 **/
	int getMaxShaderStorageBufferSize() const;

	/**
	 * Returns the maximum number of compute work groups that can be
	 * dispatched in a given dimension.
	 */
	int getMaxComputeWorkGroupsX() const;
	int getMaxComputeWorkGroupsY() const;
	int getMaxComputeWorkGroupsZ() const;

	/**
	 * Returns the maximum supported number of simultaneous render targets.
	 **/
	int getMaxRenderTargets() const;

	/**
	 * Returns the maximum supported number of MSAA sampless.
	 **/
	int getMaxSamples() const;

	/**
	 * Returns the maximum number of accessible texture units.
	 **/
	int getMaxTextureUnits() const;

	/**
	 * Returns the maximum number of shader storage buffer bindings.
	 **/
	int getMaxShaderStorageBufferBindings() const;

	/**
	 * Returns the maximum point size.
	 **/
	float getMaxPointSize() const;

	/**
	 * Returns the maximum anisotropic filtering value that can be used for
	 * Texture filtering.
	 **/
	float getMaxAnisotropy() const;

	float getMaxLODBias() const;

	/**
	 * Gets whether the context is Core Profile OpenGL 3.2+.
	 **/
	bool isCoreProfile() const;

	/**
	 * Get the GPU vendor of this OpenGL context.
	 **/
	Vendor getVendor() const;

	static GLenum getGLPrimitiveType(PrimitiveType type);
	static GLenum getGLBufferType(BufferUsage usage);
	static GLenum getGLIndexDataType(IndexDataType type);
	static GLenum getGLVertexDataType(DataFormat format, int &components, GLboolean &normalized, bool &intformat);
	static GLenum getGLBufferDataUsage(BufferDataUsage usage);
	static GLenum getGLTextureType(TextureType type);
	static GLint getGLWrapMode(SamplerState::WrapMode wmode);
	static GLint getGLCompareMode(CompareMode mode);

	static TextureFormat convertPixelFormat(PixelFormat pixelformat);
	static bool isTexStorageSupported();
	static uint32 getPixelFormatUsageFlags(PixelFormat pixelformat);

	static const char *errorString(GLenum errorcode);
	static const char *framebufferStatusString(GLenum status);

	// Get human-readable strings for debug info.
	static const char *debugSeverityString(GLenum severity);
	static const char *debugSourceString(GLenum source);
	static const char *debugTypeString(GLenum type);

private:

	void initVendor();
	void initOpenGLFunctions();
	void initMaxValues();

	bool contextInitialized;

	bool baseVertexSupported;

	float maxAnisotropy;
	float maxLODBias;
	int max2DTextureSize;
	int max3DTextureSize;
	int maxCubeTextureSize;
	int maxTextureArrayLayers;
	int maxTexelBufferSize;
	int maxShaderStorageBufferSize;
	int maxComputeWorkGroupsX;
	int maxComputeWorkGroupsY;
	int maxComputeWorkGroupsZ;
	int maxRenderTargets;
	int maxSamples;
	int maxTextureUnits;
	int maxShaderStorageBufferBindings;
	float maxPointSize;

	bool coreProfile;

	Vendor vendor;

	// Tracked OpenGL state.
	struct
	{
		GLuint boundBuffers[BUFFERUSAGE_MAX_ENUM];

		// Texture unit state (currently bound texture for each texture unit.)
		std::vector<GLuint> boundTextures[TEXTURE_MAX_ENUM + 1];

		std::vector<GLuint> boundIndexedBuffers[BUFFERUSAGE_MAX_ENUM];

		bool enableState[ENABLE_MAX_ENUM];

		GLenum faceCullMode;

		int curTextureUnit;

		uint32 enabledAttribArrays;
		uint32 instancedAttribArrays;

		Rect viewport;
		Rect scissor;

		bool depthWritesEnabled = true;
		uint32 stencilWriteMask = LOVE_UINT32_MAX;
		uint32 colorWriteMask = LOVE_UINT32_MAX;

		GLuint boundFramebuffers[2];

	} state;

}; // OpenGL

// OpenGL class instance singleton.
extern OpenGL gl;

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_OPENGL_H
