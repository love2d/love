/**
 * Copyright (c) 2006-2015 LOVE Development Team
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
	ATTRIB_MAX_ENUM
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
		VENDOR_ATI_AMD,
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

	struct BlendState
	{
		GLenum srcRGB, srcA;
		GLenum dstRGB, dstA;
		GLenum func;
	};

	struct
	{
		std::vector<Matrix> transform;
		std::vector<Matrix> projection;
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

		Matrix &get()
		{
			return gl.getTransform();
		}

	private:
		OpenGL &gl;
	};

	struct Stats
	{
		size_t textureMemory;
		int    drawCalls;
		int    framebufferBinds;
	} stats;

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
	Matrix &getTransform();

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
	 * Sets the current constant color.
	 **/
	void setColor(const Color &c);

	/**
	 * Gets the current constant color.
	 **/
	Color getColor() const;

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
	 * Sets blending functionality.
	 * Note: This does not globally enable or disable blending.
	 **/
	void setBlendState(const BlendState &blend);

	/**
	 * Gets the currently set blending functionality.
	 **/
	BlendState getBlendState() const;

	/**
	 * Sets the global point size.
	 **/
	void setPointSize(float size);

	/**
	 * Gets the global point size.
	 **/
	float getPointSize() const;

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
	 * Binds multiple textures to texture units without changing the active
	 * texture unit. Equivalent to glBindTextures.
	 **/
	void bindTextures(GLuint first, GLsizei count, const GLuint *textures);

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
		// Current constant color.
		Color color;

		// Texture unit state (currently bound texture for each texture unit.)
		std::vector<GLuint> boundTextures;

		// Currently active texture unit.
		int curTextureUnit;

		Viewport viewport;
		Viewport scissor;

		float pointSize;

		GLuint defaultTexture;

		BlendState blend;

		Matrix lastProjectionMatrix;
		Matrix lastTransformMatrix;

	} state;

}; // OpenGL

// OpenGL class instance singleton.
extern OpenGL gl;

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_OPENGL_H
