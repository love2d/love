/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "graphics/Color.h"
#include "graphics/Texture.h"
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
namespace opengl
{

// Awful, but the library uses the namespace in order to use the functions sanely
// with proper autocomplete in IDEs while having name mangling safety -
// no clashes with other GL libraries when linking, etc.
using namespace glad;

// Vertex attribute indices used in shaders by LOVE. The values map to OpenGL
// generic vertex attribute indices.
enum VertexAttribID
{
	ATTRIB_POS = 0,
	ATTRIB_TEXCOORD,
	ATTRIB_COLOR,
	ATTRIB_CONSTANTCOLOR,
	ATTRIB_MAX_ENUM
};

enum VertexAttribFlags
{
	ATTRIBFLAG_POS = 1 << ATTRIB_POS,
	ATTRIBFLAG_TEXCOORD = 1 << ATTRIB_TEXCOORD,
	ATTRIBFLAG_COLOR = 1 << ATTRIB_COLOR,
	ATTRIBFLAG_CONSTANTCOLOR = 1 << ATTRIB_CONSTANTCOLOR
};

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

	// A rectangle representing an OpenGL viewport or a scissor box.
	struct Viewport
	{
		int x, y;
		int w, h;

		bool operator == (const Viewport &rhs) const
		{
			return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
		}
	};

	struct
	{
		std::vector<Matrix4> transform;
		std::vector<Matrix4> projection;
	} matrices;

	class TempTransform
	{
	public:

		TempTransform(OpenGL &gl)
			: gl(gl)
		{
			gl.pushTransform();
		}

		~TempTransform()
		{
			gl.popTransform();
		}

		Matrix4 &get()
		{
			return gl.getTransform();
		}

	private:
		OpenGL &gl;
	};

	class TempDebugGroup
	{
	public:

#if defined(LOVE_IOS)
		TempDebugGroup(const char *name)
		{
			if (GLAD_EXT_debug_marker)
				glPushGroupMarkerEXT(0, (const GLchar *) name);
		}
#else
		TempDebugGroup(const char *) {}
#endif

#if defined(LOVE_IOS)
		~TempDebugGroup()
		{
			if (GLAD_EXT_debug_marker)
				glPopGroupMarkerEXT();
		}
#endif
	};

	struct Stats
	{
		size_t textureMemory;
		int    drawCalls;
		int    framebufferBinds;
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

	void pushTransform();
	void popTransform();
	Matrix4 &getTransform();

	/**
	 * Set up necessary state (LOVE-provided shader uniforms, etc.) for drawing.
	 * This *MUST* be called directly before OpenGL drawing functions.
	 **/
	void prepareDraw();

	/**
	 * glDrawArrays and glDrawElements which increment the draw-call counter by
	 * themselves.
	 **/
	void drawArrays(GLenum mode, GLint first, GLsizei count);
	void drawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);

	/**
	 * Sets the enabled vertex attribute arrays based on the specified attribute
	 * bits. Each bit in the uint32 represents an enabled attribute array index.
	 * For example, useVertexAttribArrays(1 << 0) will enable attribute index 0.
	 * See the VertexAttribFlags enum for the standard vertex attributes.
	 * This function *must* be used instead of glEnable/DisableVertexAttribArray.
	 **/
	void useVertexAttribArrays(uint32 arraybits);

	/**
	 * Sets the OpenGL rendering viewport to the specified rectangle.
	 * The y-coordinate starts at the top.
	 **/
	void setViewport(const Viewport &v);

	/**
	 * Gets the current OpenGL rendering viewport rectangle.
	 **/
	Viewport getViewport() const;

	/**
	 * Sets the scissor box to the specified rectangle.
	 * The y-coordinate starts at the top and is flipped internally.
	 **/
	void setScissor(const Viewport &v);

	/**
	 * Gets the current scissor box (regardless of whether scissoring is enabled.)
	 **/
	Viewport getScissor() const;

	/**
	 * Sets the global point size.
	 **/
	void setPointSize(float size);

	/**
	 * Gets the global point size.
	 **/
	float getPointSize() const;

	/**
	 * Calls glEnable/glDisable(GL_FRAMEBUFFER_SRGB).
	 **/
	void setFramebufferSRGB(bool enable);

	/**
	 * Equivalent to glIsEnabled(GL_FRAMEBUFFER_SRGB).
	 **/
	bool hasFramebufferSRGB() const;

	/**
	 * Binds a Framebuffer Object to the specified target.
	 **/
	void bindFramebuffer(GLenum target, GLuint framebuffer);

	/**
	 * This will usually be 0 (system drawable), but some platforms require a
	 * non-zero FBO for rendering.
	 **/
	GLuint getDefaultFBO() const;

	/**
	 * Gets the ID for love's default texture (used for "untextured" primitives.)
	 **/
	GLuint getDefaultTexture() const;

	/**
	 * Helper for setting the active texture unit.
	 *
	 * @param textureunit Index in the range of [0, maxtextureunits-1]
	 **/
	void setTextureUnit(int textureunit);

	/**
	 * Helper for binding an OpenGL texture.
	 * Makes sure we aren't redundantly binding textures.
	 **/
	void bindTexture(GLuint texture);

	/**
	 * Helper for binding a texture to a specific texture unit.
	 *
	 * @param textureunit Index in the range of [0, maxtextureunits-1]
	 * @param restoreprev Restore previously bound texture unit when done.
	 **/
	void bindTextureToUnit(GLuint texture, int textureunit, bool restoreprev);

	/**
	 * Helper for deleting an OpenGL texture.
	 * Cleans up if the texture is currently bound.
	 **/
	void deleteTexture(GLuint texture);

	/**
	 * Sets the texture filter mode for the currently bound texture.
	 * The anisotropy parameter of the argument is set to the actual amount of
	 * anisotropy that was used.
	 **/
	void setTextureFilter(graphics::Texture::Filter &f);

	/**
	 * Sets the texture wrap mode for the currently bound texture.
	 **/
	void setTextureWrap(const graphics::Texture::Wrap &w);

	bool isClampZeroTextureWrapSupported() const;

	/**
	 * Returns the maximum supported width or height of a texture.
	 **/
	int getMaxTextureSize() const;

	/**
	 * Returns the maximum supported number of simultaneous render targets.
	 **/
	int getMaxRenderTargets() const;

	/**
	 * Returns the maximum supported number of MSAA samples for renderbuffers.
	 **/
	int getMaxRenderbufferSamples() const;

	/**
	 * Returns the maximum number of accessible texture units.
	 **/
	int getMaxTextureUnits() const;

	void updateTextureMemorySize(size_t oldsize, size_t newsize);

	/**
	 * Get the GPU vendor of this OpenGL context.
	 **/
	Vendor getVendor() const;

	static const char *errorString(GLenum errorcode);

	// Get human-readable strings for debug info.
	static const char *debugSeverityString(GLenum severity);
	static const char *debugSourceString(GLenum source);
	static const char *debugTypeString(GLenum type);

private:

	void initVendor();
	void initOpenGLFunctions();
	void initMaxValues();
	void initMatrices();
	void createDefaultTexture();

	GLint getGLWrapMode(Texture::WrapMode wmode);

	bool contextInitialized;

	float maxAnisotropy;
	int maxTextureSize;
	int maxRenderTargets;
	int maxRenderbufferSamples;
	int maxTextureUnits;

	Vendor vendor;

	// Tracked OpenGL state.
	struct
	{
		// Texture unit state (currently bound texture for each texture unit.)
		std::vector<GLuint> boundTextures;

		// Currently active texture unit.
		int curTextureUnit;

		uint32 enabledAttribArrays;

		Viewport viewport;
		Viewport scissor;

		float pointSize;

		bool framebufferSRGBEnabled;

		GLuint defaultTexture;

		Matrix4 lastProjectionMatrix;
		Matrix4 lastTransformMatrix;

	} state;

}; // OpenGL

// OpenGL class instance singleton.
extern OpenGL gl;

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_OPENGL_H
