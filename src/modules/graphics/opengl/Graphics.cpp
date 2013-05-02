/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "common/config.h"
#include "common/math.h"
#include "common/Vector.h"

#include "Graphics.h"
#include "window/sdl/Window.h"

#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

using love::window::WindowFlags;

namespace love
{
namespace graphics
{
namespace opengl
{

Graphics::Graphics()
	: currentFont(0)
	, lineStyle(LINE_SMOOTH)
	, lineWidth(1)
	, matrixLimit(0)
	, userMatrices(0)
	, colorMask()
{
	currentWindow = love::window::sdl::Window::getSingleton();
}

Graphics::~Graphics()
{
	if (currentFont != 0)
		currentFont->release();

	currentWindow->release();
}

const char *Graphics::getName() const
{
	return "love.graphics.opengl";
}

bool Graphics::checkMode(int width, int height, bool fullscreen) const
{
	return currentWindow->checkWindowSize(width, height, fullscreen);
}

DisplayState Graphics::saveState()
{
	DisplayState s;

	s.color = getColor();
	s.backgroundColor = getBackgroundColor();

	s.blendMode = getBlendMode();
	//get line style
	s.lineStyle = lineStyle;
	//get the point size
	glGetFloatv(GL_POINT_SIZE, &s.pointSize);
	//get point style
	s.pointStyle = (glIsEnabled(GL_POINT_SMOOTH) == GL_TRUE) ? Graphics::POINT_SMOOTH : Graphics::POINT_ROUGH;
	// get alpha test status
	s.alphaTest = isAlphaTestEnabled();
	if (s.alphaTest)
	{
		// if alpha testing is enabled, store mode and reference alpha
		s.alphaTestMode = getAlphaTestMode();
		s.alphaTestRef = getAlphaTestRef();
	}
	//get scissor status
	s.scissor = (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
	//do we have scissor, if so, store the box
	if (s.scissor)
		glGetIntegerv(GL_SCISSOR_BOX, s.scissorBox);

	for (int i = 0; i < 4; i++)
		s.colorMask[i] = colorMask[i];

	return s;
}

void Graphics::restoreState(const DisplayState &s)
{
	setColor(s.color);
	setBackgroundColor(s.backgroundColor);
	setBlendMode(s.blendMode);
	setLine(lineWidth, s.lineStyle);
	setPoint(s.pointSize, s.pointStyle);
	if (s.alphaTest)
		setAlphaTest(s.alphaTestMode, s.alphaTestRef);
	else
		setAlphaTest();
	if (s.scissor)
		setScissor(s.scissorBox[0], s.scissorBox[1], s.scissorBox[2], s.scissorBox[3]);
	else
		setScissor();
	setColorMask(s.colorMask[0], s.colorMask[1], s.colorMask[2], s.colorMask[3]);
}

static void APIENTRY myErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, void *ud)
{
	(void)ud;
	std::cerr << "source = " << source << ", type = " << type << ", id = " << id << ", severity = " << severity << ", length = " << length << "\n" << message << std::endl;
}

bool Graphics::setMode(int width, int height, WindowFlags *flags)
{
	// This operation destroys the OpenGL context, so
	// we must save the state.
	DisplayState tempState;
	if (isCreated())
		tempState = saveState();

	// Unload all volatile objects. These must be reloaded after
	// the display mode change.
	Volatile::unloadAll();

	gl.deInitContext();

	bool success = currentWindow->setWindow(width, height, flags);

	// Regardless of failure, we'll have to set up OpenGL once again.
	width = currentWindow->getWidth();
	height = currentWindow->getHeight();

	// Okay, setup OpenGL.
	gl.initContext();

	// Make sure antialiasing works when set elsewhere
	if (GLEE_VERSION_1_3 || GLEE_ARB_multisample)
		glEnable(GL_MULTISAMPLE);

	// Enable blending
	glEnable(GL_BLEND);

	// "Normal" blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable all color component writes.
	setColorMask(true, true, true, true);

	// Enable line/point smoothing.
	setLineStyle(LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

	// Auto-generated mipmaps should be the best quality possible
	if (GLEE_VERSION_1_4 || GLEE_SGIS_generate_mipmap)
		glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	// Set default alpha test mode and value
	glAlphaFunc(GL_GREATER, 0.0f);
	setAlphaTest();

	// Enable textures
	glEnable(GL_TEXTURE_2D);
	gl.setActiveTextureUnit(0);

	// Set the viewport to top-left corner
	glViewport(0, 0, width, height);

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up orthographic view (no depth)
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);

	// Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set pixel row alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);

	// Reload all volatile objects.
	if (!Volatile::loadAll())
		std::cerr << "Could not reload all volatile objects." << std::endl;

	// Restore the display state.
	restoreState(tempState);

	// Get the maximum number of matrices
	// subtract a few to give the engine some room.
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &matrixLimit);
	matrixLimit -= 5;

	// Debug output is temporarily disabled until we either improve it and make
	// it optional or remove it.
	if (GLEE_KHR_debug && false)
	{
		std::cerr << "debug on" << std::endl;
		glDebugMessageCallback(myErrorCallback, NULL);
		glEnable(GL_DEBUG_OUTPUT);
	}

	return success;
}

void Graphics::getMode(int &width, int &height, WindowFlags &flags) const
{
	currentWindow->getWindow(width, height, flags);
}

bool Graphics::toggleFullscreen()
{
	int width, height;
	WindowFlags flags;
	currentWindow->getWindow(width, height, flags);
	flags.fullscreen = !flags.fullscreen;
	return setMode(width, height, &flags);
}


void Graphics::reset()
{
	DisplayState s;
	discardStencil();
	Canvas::bindDefaultCanvas();
	Shader::detach();
	restoreState(s);
}

void Graphics::clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Graphics::present()
{
	currentWindow->swapBuffers();
}

void Graphics::setIcon(Image *image)
{
	if (image->isCompressed())
		throw love::Exception("Cannot use compressed image data to set an icon.");

	currentWindow->setIcon(image->getData());
}

void Graphics::setCaption(const char *caption)
{
	std::string title(caption);
	currentWindow->setWindowTitle(title);
}

std::string Graphics::getCaption() const
{
	return currentWindow->getWindowTitle();
}

int Graphics::getWidth() const
{
	return currentWindow->getWidth();
}

int Graphics::getHeight() const
{
	return currentWindow->getHeight();
}

int Graphics::getRenderWidth() const
{
	if (Canvas::current)
		return Canvas::current->getWidth();
	return getWidth();
}

int Graphics::getRenderHeight() const
{
	if (Canvas::current)
		return Canvas::current->getHeight();
	return getHeight();
}

bool Graphics::isCreated() const
{
	return currentWindow->isCreated();
}

int Graphics::getModes(lua_State *L) const
{
	int n;
	love::window::Window::WindowSize *modes = currentWindow->getFullscreenSizes(n);

	if (modes == 0)
		return 0;

	lua_createtable(L, n, 0);

	for (int i = 0; i < n ; i++)
	{
		lua_pushinteger(L, i+1);
		lua_createtable(L, 0, 2);

		// Inner table attribs.

		lua_pushinteger(L, modes[i].width);
		lua_setfield(L, -2, "width");

		lua_pushinteger(L, modes[i].height);
		lua_setfield(L, -2, "height");

		// Inner table attribs end.

		lua_settable(L, -3);
	}

	delete[] modes;
	return 1;
}

void Graphics::setScissor(int x, int y, int width, int height)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, getRenderHeight() - (y + height), width, height); // Compensates for the fact that our y-coordinate is reverse of OpenGLs.
}

void Graphics::setScissor()
{
	glDisable(GL_SCISSOR_TEST);
}

int Graphics::getScissor(lua_State *L) const
{
	if (glIsEnabled(GL_SCISSOR_TEST) == GL_FALSE)
		return 0;

	GLint scissor[4];
	glGetIntegerv(GL_SCISSOR_BOX, scissor);

	lua_pushnumber(L, scissor[0]);
	lua_pushnumber(L, getRenderHeight() - (scissor[1] + scissor[3])); // Compensates for the fact that our y-coordinate is reverse of OpenGLs.
	lua_pushnumber(L, scissor[2]);
	lua_pushnumber(L, scissor[3]);

	return 4;
}

void Graphics::defineStencil()
{
	// Disable color writes but don't save the mask values.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void Graphics::useStencil(bool invert)
{
	glStencilFunc(GL_EQUAL, (int)(!invert), 1); // invert ? 0 : 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	setColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
}

void Graphics::discardStencil()
{
	setColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
	glDisable(GL_STENCIL_TEST);
}

void Graphics::setAlphaTest(Graphics::AlphaTestMode mode, unsigned char refalpha)
{
	GLclampf ref = refalpha / 255.0f;

	glEnable(GL_ALPHA_TEST);

	switch (mode)
	{
	case ALPHATEST_LESS:
		glAlphaFunc(GL_LESS, ref);
		break;
	case ALPHATEST_LEQUAL:
		glAlphaFunc(GL_LEQUAL, ref);
		break;
	case ALPHATEST_EQUAL:
		glAlphaFunc(GL_EQUAL, ref);
		break;
	case ALPHATEST_NOTEQUAL:
		glAlphaFunc(GL_NOTEQUAL, ref);
		break;
	case ALPHATEST_GEQUAL:
		glAlphaFunc(GL_GEQUAL, ref);
		break;
	case ALPHATEST_GREATER:
		glAlphaFunc(GL_GREATER, ref);
		break;
	default:
		glDisable(GL_ALPHA_TEST);
		break;
	}
}

void Graphics::setAlphaTest()
{
	glDisable(GL_ALPHA_TEST);
}

bool Graphics::isAlphaTestEnabled()
{
	return glIsEnabled(GL_ALPHA_TEST) == GL_TRUE;
}

Graphics::AlphaTestMode Graphics::getAlphaTestMode()
{
	GLint func;
	glGetIntegerv(GL_ALPHA_TEST_FUNC, &func);

	switch (func)
	{
	case GL_LESS:
		return ALPHATEST_LESS;
	case GL_LEQUAL:
		return ALPHATEST_LEQUAL;
	case GL_EQUAL:
		return ALPHATEST_EQUAL;
	case GL_NOTEQUAL:
		return ALPHATEST_NOTEQUAL;
	case GL_GEQUAL:
		return ALPHATEST_GEQUAL;
	case GL_GREATER:
		return ALPHATEST_GREATER;
	default:
		return ALPHATEST_MAX_ENUM;
	}
}

unsigned char Graphics::getAlphaTestRef()
{
	GLfloat ref;
	glGetFloatv(GL_ALPHA_TEST_REF, &ref);

	return ref * 255;
}

Image *Graphics::newImage(love::image::ImageData *data)
{
	// Create the image.
	Image *image = new Image(data);
	bool success;
	try
	{
		success = image->load();
	}
	catch(love::Exception &)
	{
		image->release();
		throw;
	}
	if (!success)
	{
		image->release();
		return 0;
	}

	return image;
}

Image *Graphics::newImage(love::image::CompressedData *cdata)
{
	// Create the image.
	Image *image = new Image(cdata);
	bool success;
	try
	{
		success = image->load();
	}
	catch(love::Exception &)
	{
		image->release();
		throw;
	}
	if (!success)
	{
		image->release();
		return 0;
	}

	return image;
}

Geometry *Graphics::newGeometry(const std::vector<vertex> &vertices)
{
	return new Geometry(vertices);
}

Geometry *Graphics::newQuad(float x, float y, float w, float h, float sw, float sh)
{
	return new Geometry(x, y, w, h, sw, sh);
}

Font *Graphics::newFont(love::font::Rasterizer *r, const Image::Filter &filter)
{
	return new Font(r, filter);
}

SpriteBatch *Graphics::newSpriteBatch(Image *image, int size, int usage)
{
	return new SpriteBatch(image, size, usage);
}

ParticleSystem *Graphics::newParticleSystem(Image *image, int size)
{
	return new ParticleSystem(image, size);
}

Canvas *Graphics::newCanvas(int width, int height, Canvas::TextureType texture_type)
{
	if (texture_type == Canvas::TYPE_HDR && !Canvas::isHDRSupported())
		throw Exception("HDR Canvases are not supported by your OpenGL implementation");

	while (GL_NO_ERROR != glGetError())
		/* clear opengl error flag */;

	Canvas *canvas = new Canvas(width, height, texture_type);
	GLenum err = canvas->getStatus();

	// everything ok, return canvas (early out)
	if (err == GL_FRAMEBUFFER_COMPLETE)
		return canvas;

	// create error message
	std::stringstream error_string;
	error_string << "Cannot create canvas: ";
	switch (err)
	{

	case GL_FRAMEBUFFER_UNSUPPORTED:
		error_string << "Not supported by your OpenGL implementation.";
		break;

		// remaining error codes are highly unlikely:
	case GL_FRAMEBUFFER_UNDEFINED:
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		error_string << "Error in implementation. Possible fix: Make canvas width and height powers of two.";
		break;

	default:
		// my intel hda card wrongly returns 0 to glCheckFramebufferStatus() but sets
		// no error flag. I think it meant to return GL_FRAMEBUFFER_UNSUPPORTED, but who
		// knows.
		if (glGetError() == GL_NO_ERROR)
			error_string << "May not be supported by your OpenGL implementation.";
		// the remaining error is an indication of a serious fuckup since it should
		// only be returned if glCheckFramebufferStatus() was called with the wrong
		// arguments.
		else
			error_string << "Cannot create canvas: Aliens did it (OpenGL error code: " << glGetError() << ")";
	}

	canvas->release();
	throw Exception(error_string.str().c_str());
	return NULL; // never reached
}

Shader *Graphics::newShader(const Shader::ShaderSources &sources)
{
	return new Shader(sources);
}

void Graphics::setColor(const Color &c)
{
	gl.setColor(c);
}

Color Graphics::getColor() const
{
	return gl.getColor();
}

void Graphics::setBackgroundColor(const Color &c)
{
	glClearColor((float)c.r/255.0f, (float)c.g/255.0f, (float)c.b/255.0f, (float)c.a/255.0f);
}

Color Graphics::getBackgroundColor() const
{
	float c[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, c);

	Color t;
	t.r = (unsigned char)(255.0f*c[0]);
	t.g = (unsigned char)(255.0f*c[1]);
	t.b = (unsigned char)(255.0f*c[2]);
	t.a = (unsigned char)(255.0f*c[3]);

	return t;
}

void Graphics::setFont(Font *font)
{
	if (currentFont != 0)
		currentFont->release();

	currentFont = font;

	if (font != 0)
		currentFont->retain();
}

Font *Graphics::getFont() const
{
	return currentFont;
}

void Graphics::setColorMask(bool r, bool g, bool b, bool a)
{
	colorMask[0] = r;
	colorMask[1] = g;
	colorMask[2] = b;
	colorMask[3] = a;

	glColorMask((GLboolean) r, (GLboolean) g, (GLboolean) b, (GLboolean) a);
}

const bool *Graphics::getColorMask() const
{
	return colorMask;
}

void Graphics::setBlendMode(Graphics::BlendMode mode)
{
	const int gl_1_4 = GLEE_VERSION_1_4;

	GLenum func = GL_FUNC_ADD;
	GLenum src_rgb = GL_ONE;
	GLenum src_a = GL_ONE;
	GLenum dst_rgb = GL_ZERO;
	GLenum dst_a = GL_ZERO;

	switch (mode)
	{
	case BLEND_ALPHA:
		if (gl_1_4 || GLEE_EXT_blend_func_separate)
		{
			src_rgb = GL_SRC_ALPHA;
			src_a = GL_ONE;
			dst_rgb = dst_a = GL_ONE_MINUS_SRC_ALPHA;
		}
		else
		{
			// Fallback for OpenGL implementations without support for separate blend functions.
			// This will most likely only be used for the Microsoft software renderer and
			// since it's still stuck with OpenGL 1.1, the only expected difference is a
			// different alpha value when reading back the default framebuffer (newScreenshot).
			src_rgb = src_a = GL_SRC_ALPHA;
			dst_rgb = dst_a = GL_ONE_MINUS_SRC_ALPHA;
		}
		break;
	case BLEND_MULTIPLICATIVE:
		src_rgb = src_a = GL_DST_COLOR;
		dst_rgb = dst_a = GL_ZERO;
		break;
	case BLEND_PREMULTIPLIED:
		src_rgb = src_a = GL_ONE;
		dst_rgb = dst_a = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_SUBTRACTIVE:
		func = GL_FUNC_REVERSE_SUBTRACT;
	case BLEND_ADDITIVE:
		src_rgb = src_a = GL_SRC_ALPHA;
		dst_rgb = dst_a = GL_ONE;
		break;
	case BLEND_NONE:
	default:
		src_rgb = src_a = GL_ONE;
		dst_rgb = dst_a = GL_ZERO;
		break;
	}

	if (gl_1_4 || GLEE_ARB_imaging)
		glBlendEquation(func);
	else if (GLEE_EXT_blend_minmax && GLEE_EXT_blend_subtract)
		glBlendEquationEXT(func);
	else
	{
		if (func == GL_FUNC_REVERSE_SUBTRACT)
			throw Exception("This graphics card does not support the subtractive blend mode!");
		// GL_FUNC_ADD is the default even without access to glBlendEquation, so that'll still work.
	}

	if (src_rgb == src_a && dst_rgb == dst_a)
		glBlendFunc(src_rgb, dst_rgb);
	else
	{
		if (gl_1_4)
			glBlendFuncSeparate(src_rgb, dst_rgb, src_a, dst_a);
		else if (GLEE_EXT_blend_func_separate)
			glBlendFuncSeparateEXT(src_rgb, dst_rgb, src_a, dst_a);
		else
			throw Exception("This graphics card does not support separated rgb and alpha blend functions!");
	}
}

Graphics::BlendMode Graphics::getBlendMode() const
{
	const int gl_1_4 = GLEE_VERSION_1_4;

	GLint src_rgb, src_a, dst_rgb, dst_a;
	GLint equation = GL_FUNC_ADD;

	if (gl_1_4 || GLEE_EXT_blend_func_separate)
	{
		glGetIntegerv(GL_BLEND_SRC_RGB, &src_rgb);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &src_a);
		glGetIntegerv(GL_BLEND_DST_RGB, &dst_rgb);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &dst_a);
	}
	else
	{
		glGetIntegerv(GL_BLEND_SRC, &src_rgb);
		glGetIntegerv(GL_BLEND_DST, &dst_rgb);
		src_a = src_rgb;
		dst_a = dst_rgb;
	}

	if (gl_1_4 || GLEE_ARB_imaging || (GLEE_EXT_blend_minmax && GLEE_EXT_blend_subtract))
		glGetIntegerv(GL_BLEND_EQUATION, &equation);

	if (equation == GL_FUNC_REVERSE_SUBTRACT)  // && src == GL_SRC_ALPHA && dst == GL_ONE
		return BLEND_SUBTRACTIVE;
	// Everything else has equation == GL_FUNC_ADD.
	else if (src_rgb == src_a && dst_rgb == dst_a)
	{
		if (src_rgb == GL_SRC_ALPHA && dst_rgb == GL_ONE)
			return BLEND_ADDITIVE;
		else if (src_rgb == GL_SRC_ALPHA && dst_rgb == GL_ONE_MINUS_SRC_ALPHA)
			return BLEND_ALPHA; // alpha blend mode fallback for very old OpenGL versions.
		else if (src_rgb == GL_DST_COLOR && dst_rgb == GL_ZERO)
			return BLEND_MULTIPLICATIVE;
		else if (src_rgb == GL_ONE && dst_rgb == GL_ONE_MINUS_SRC_ALPHA)
			return BLEND_PREMULTIPLIED;
		else if (src_rgb == GL_ONE && dst_rgb == GL_ZERO)
			return BLEND_NONE;
	}
	else if (src_rgb == GL_SRC_ALPHA && src_a == GL_ONE &&
		dst_rgb == GL_ONE_MINUS_SRC_ALPHA && dst_a == GL_ONE_MINUS_SRC_ALPHA)
		return BLEND_ALPHA;

	throw Exception("Unknown blend mode");
}

void Graphics::setDefaultFilter(const Image::Filter &f)
{
	Image::setDefaultFilter(f);
}

const Image::Filter &Graphics::getDefaultFilter() const
{
	return Image::getDefaultFilter();
}

void Graphics::setDefaultMipmapFilter(Image::FilterMode filter, float sharpness)
{
	Image::setDefaultMipmapFilter(filter);
	Image::setDefaultMipmapSharpness(sharpness);
}

void Graphics::getDefaultMipmapFilter(Image::FilterMode *filter, float *sharpness) const
{
	*filter = Image::getDefaultMipmapFilter();
	*sharpness = Image::getDefaultMipmapSharpness();
}

void Graphics::setLineWidth(float width)
{
	lineWidth = width;
}

void Graphics::setLineStyle(Graphics::LineStyle style)
{
	lineStyle = style;
}

void Graphics::setLine(float width, Graphics::LineStyle style)
{
	setLineWidth(width);

	if (style == 0)
		return;
	setLineStyle(style);
}

float Graphics::getLineWidth() const
{
	return lineWidth;
}

Graphics::LineStyle Graphics::getLineStyle() const
{
	return lineStyle;
}

void Graphics::setPointSize(float size)
{
	glPointSize((GLfloat)size);
}

void Graphics::setPointStyle(Graphics::PointStyle style)
{
	if (style == POINT_SMOOTH)
		glEnable(GL_POINT_SMOOTH);
	else // love::POINT_ROUGH
		glDisable(GL_POINT_SMOOTH);
}

void Graphics::setPoint(float size, Graphics::PointStyle style)
{
	if (style == POINT_SMOOTH)
		glEnable(GL_POINT_SMOOTH);
	else // POINT_ROUGH
		glDisable(GL_POINT_SMOOTH);

	glPointSize((GLfloat)size);
}

float Graphics::getPointSize() const
{
	GLfloat size;
	glGetFloatv(GL_POINT_SIZE, &size);
	return (float)size;
}

Graphics::PointStyle Graphics::getPointStyle() const
{
	if (glIsEnabled(GL_POINT_SMOOTH) == GL_TRUE)
		return POINT_SMOOTH;
	else
		return POINT_ROUGH;
}

int Graphics::getMaxPointSize() const
{
	GLint max;
	glGetIntegerv(GL_POINT_SIZE_MAX, &max);
	return (int)max;
}

void Graphics::print(const char *str, float x, float y , float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	if (currentFont != 0)
	{
		std::string text(str);
		currentFont->print(text, x, y, 0.0, angle, sx, sy, ox, oy, kx, ky);
	}
}

void Graphics::printf(const char *str, float x, float y, float wrap, AlignMode align, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	if (currentFont == 0)
		return;

	using std::string;
	using std::vector;

	string text(str);
	vector<string> lines_to_draw = currentFont->getWrap(text, wrap);

	glPushMatrix();

	static Matrix t;
	t.setTransformation(ceil(x), ceil(y), angle, sx, sy, ox, oy, kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());

	x = y = 0.0f;

	try
	{
		// now for the actual printing
		vector<string>::const_iterator line_iter, line_end = lines_to_draw.end();
		float letter_spacing = 0.0f;
		for (line_iter = lines_to_draw.begin(); line_iter != line_end; ++line_iter)
		{
			float width = static_cast<float>(currentFont->getWidth(*line_iter));
			switch (align)
			{
			case ALIGN_RIGHT:
				currentFont->print(*line_iter, ceil(x + (wrap - width)), ceil(y), 0.0f);
				break;
			case ALIGN_CENTER:
				currentFont->print(*line_iter, ceil(x + (wrap - width) / 2), ceil(y), 0.0f);
				break;
			case ALIGN_JUSTIFY:
				if (line_iter->length() > 1 && (line_iter+1) != line_end)
					letter_spacing = (wrap - width) / float(line_iter->length() - 1);
				else
					letter_spacing = 0.0f;
				currentFont->print(*line_iter, ceil(x), ceil(y), letter_spacing);
				break;
			case ALIGN_LEFT:
			default:
				currentFont->print(*line_iter, ceil(x), ceil(y), 0.0f);
				break;
			}
			y += currentFont->getHeight() * currentFont->getLineHeight();
		}
	}
	catch (love::Exception &)
	{
		glPopMatrix();
		throw;
	}

	glPopMatrix();
}

/**
 * Primitives
 **/

void Graphics::point(float x, float y)
{
	gl.bindTexture(0);
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();
}

// Calculate line boundary points u1 and u2. Sketch:
//              u1
// -------------+---...___
//              |         ```'''--  ---
// p- - - - - - q- - . _ _           | w/2
//              |          ` ' ' r   +
// -------------+---...___           | w/2
//              u2         ```'''-- ---
//
// u1 and u2 depend on four things:
//   - the half line width w/2
//   - the previous line vertex p
//   - the current line vertex q
//   - the next line vertex r
//
// u1/u2 are the intersection points of the parallel lines to p-q and q-r,
// i.e. the point where
//
//    (p + w/2 * n1) + mu * (q - p) = (q + w/2 * n2) + lambda * (r - q)   (u1)
//    (p - w/2 * n1) + mu * (q - p) = (q - w/2 * n2) + lambda * (r - q)   (u2)
//
// with n1,n2 being the normals on the segments p-q and q-r:
//
//    n1 = perp(q - p) / |q - p|
//    n2 = perp(r - q) / |r - q|
//
// The intersection points can be calculated using cramers rule.
static void pushIntersectionPoints(Vector *vertices, Vector *overdraw,
								   int pos, int count, float hw, float overdraw_factor,
								   const Vector &p, const Vector &q, const Vector &r)
{
	// calculate line directions
	Vector s = (q - p);
	Vector t = (r - q);

	// calculate vertex displacement vectors
	Vector n1 = s.getNormal();
	Vector n2 = t.getNormal();
	n1.normalize();
	n2.normalize();
	float det_norm = n1 ^ n2; // will be close to zero if the angle between the normals is sharp
	n1 *= hw;
	n2 *= hw;

	// lines parallel -> assume intersection at displacement points
	if (fabs(det_norm) <= .03)
	{
		vertices[pos]   = q - n2;
		vertices[pos+1] = q + n2;
	}
	// real intersection -> calculate boundary intersection points with cramers rule
	else
	{
		float det = s ^ t;
		Vector d = n1 - n2;
		Vector b = s - d; // s = q - p
		Vector c = s + d;
		float lambda = (b ^ t) / det;
		float mu     = (c ^ t) / det;

		// ordering for GL_TRIANGLE_STRIP
		vertices[pos]   = p + s*mu - n1;     // u1
		vertices[pos+1] = p + s*lambda + n1; // u2
	}

	if (overdraw)
	{
		// displacement of the overdraw vertices
		Vector x = (vertices[pos] - q) * overdraw_factor;

		overdraw[pos]   = vertices[pos];
		overdraw[pos+1] = vertices[pos] + x;

		overdraw[2*count-pos-2] = vertices[pos+1];
		overdraw[2*count-pos-1] = vertices[pos+1] - x;
	}
}

// precondition:
// glEnableClientState(GL_VERTEX_ARRAY);
static void draw_overdraw(Vector *overdraw, size_t count, float pixel_size, bool looping)
{
	// if not looping, the outer overdraw vertices need to be displaced
	// to cover the line endings, i.e.:
	// +- - - - //- - +         +- - - - - //- - - +
	// +-------//-----+         : +-------//-----+ :
	// | core // line |   -->   : | core // line | :
	// +-----//-------+         : +-----//-------+ :
	// +- - //- - - - +         +- - - //- - - - - +
	if (!looping)
	{
		Vector s = overdraw[1] - overdraw[3];
		s.normalize();
		s *= pixel_size;
		overdraw[1] += s;
		overdraw[2*count-1] += s;

		Vector t = overdraw[count-1] - overdraw[count-3];
		t.normalize();
		t *= pixel_size;
		overdraw[count-1] += t;
		overdraw[count+1] += t;

		// we need to draw two more triangles to close the
		// overdraw at the line start.
		overdraw[2*count]   = overdraw[0];
		overdraw[2*count+1] = overdraw[1];
	}

	// prepare colors:
	// even indices in overdraw* point to inner vertices => alpha = current-alpha,
	// odd indices point to outer vertices => alpha = 0.
	Color c = gl.getColor();

	Color *colors = new Color[2*count+2];
	for (size_t i = 0; i < 2*count+2; ++i)
	{
		colors[i] = c;
		// avoids branching. equiv to if (i%2 == 1) colors[i].a = 0;
		colors[i].a *= GLubyte(i % 2 == 0);
	}

	// draw faded out line halos
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	glVertexPointer(2, GL_FLOAT, 0, (const GLvoid *)overdraw);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*count + 2 * int(!looping));
	glDisableClientState(GL_COLOR_ARRAY);
	// "if GL_COLOR_ARRAY is enabled, the value of the current color is
	// undefined after glDrawArrays executes"
	
	gl.setColor(c);

	delete[] colors;
}

void Graphics::polyline(const float *coords, size_t count)
{
	Vector *vertices = new Vector[count]; // two vertices for every line end-point
	Vector *overdraw = NULL;

	Vector p,q,r;
	bool looping = (coords[0] == coords[count-2]) && (coords[1] == coords[count-1]);

	float halfwidth       = lineWidth/2.f;
	float pixel_size      = 1.f;
	float overdraw_factor = .0f;

	if (lineStyle == LINE_SMOOTH)
	{
		overdraw = new Vector[2*count+2];
		// TODO: is there a better way to get the pixel size at the current scale?
		GLfloat m[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, m);
		float det  = m[0]*m[5]*m[10] + m[4]*m[9]*m[2] + m[8]*m[1]*m[6];
		det       -= m[2]*m[5]*m[8]  + m[6]*m[9]*m[0] + m[10]*m[1]*m[4];
		pixel_size = 1.f / sqrtf(det);

		overdraw_factor = pixel_size / halfwidth;
		halfwidth = std::max(.0f, halfwidth - .25f*pixel_size);
	}

	// get line vertex boundaries
	// if not looping, extend the line at the beginning, else use last point as `p'
	r = Vector(coords[0], coords[1]);
	if (!looping)
		q = r * 2 - Vector(coords[2], coords[3]);
	else
		q = Vector(coords[count-4], coords[count-3]);

	for (size_t i = 0; i+3 < count; i += 2)
	{
		p = q;
		q = r;
		r = Vector(coords[i+2], coords[i+3]);
		pushIntersectionPoints(vertices, overdraw, i, count, halfwidth, overdraw_factor, p,q,r);
	}

	// if not looping, extend the line at the end, else use first point as `r'
	p = q;
	q = r;
	if (!looping)
		r += q - p;
	else
		r = Vector(coords[2], coords[3]);
	pushIntersectionPoints(vertices, overdraw, count-2, count, halfwidth, overdraw_factor, p,q,r);
	// end get line vertex boundaries

	// draw the core line
	gl.bindTexture(0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, (const GLvoid *)vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, count);

	// draw the line halo (antialiasing)
	if (lineStyle == LINE_SMOOTH)
		draw_overdraw(overdraw, count, pixel_size, looping);

	glDisableClientState(GL_VERTEX_ARRAY);

	// cleanup
	delete[] vertices;
	if (lineStyle == LINE_SMOOTH)
		delete[] overdraw;
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h)
{
	float coords[] = {x,y, x,y+h, x+w,y+h, x+w,y, x,y};
	polygon(mode, coords, 5 * 2);
}

void Graphics::circle(DrawMode mode, float x, float y, float radius, int points)
{
	float two_pi = static_cast<float>(LOVE_M_PI * 2);
	if (points <= 0) points = 1;
	float angle_shift = (two_pi / points);
	float phi = .0f;

	float *coords = new float[2 * (points + 1)];
	for (int i = 0; i < points; ++i, phi += angle_shift)
	{
		coords[2*i]   = x + radius * cosf(phi);
		coords[2*i+1] = y + radius * sinf(phi);
	}

	coords[2*points]   = coords[0];
	coords[2*points+1] = coords[1];

	polygon(mode, coords, (points + 1) * 2);

	delete[] coords;
}

void Graphics::arc(DrawMode mode, float x, float y, float radius, float angle1, float angle2, int points)
{
	// Nothing to display with no points or equal angles. (Or is there with line mode?)
	if (points <= 0 || angle1 == angle2)
		return;

	// Oh, you want to draw a circle?
	if (fabs(angle1 - angle2) >= 2.0f * (float) LOVE_M_PI)
	{
		circle(mode, x, y, radius, points);
		return;
	}

	float angle_shift = (angle2 - angle1) / points;
	// Bail on precision issues.
	if (angle_shift == 0.0)
		return;

	float phi = angle1;
	int num_coords = (points + 3) * 2;
	float *coords = new float[num_coords];
	coords[0] = coords[num_coords - 2] = x;
	coords[1] = coords[num_coords - 1] = y;

	for (int i = 0; i <= points; ++i, phi += angle_shift)
	{
		coords[2 * (i+1)]     = x + radius * cosf(phi);
		coords[2 * (i+1) + 1] = y + radius * sinf(phi);
	}

	// GL_POLYGON can only fill-draw convex polygons, so we need to do stuff manually here
	if (mode == DRAW_LINE)
	{
		polyline(coords, num_coords); // Artifacts at sharp angles if set to looping.
	}
	else
	{
		gl.bindTexture(0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, (const GLvoid *) coords);
		glDrawArrays(GL_TRIANGLE_FAN, 0, points + 2);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	delete[] coords;
}

/// @param mode    the draw mode
/// @param coords  the coordinate array
/// @param count   the number of coordinates/size of the array
void Graphics::polygon(DrawMode mode, const float *coords, size_t count)
{
	// coords is an array of a closed loop of vertices, i.e.
	// coords[count-2] = coords[0], coords[count-1] = coords[1]
	if (mode == DRAW_LINE)
	{
		polyline(coords, count);
	}
	else
	{
		gl.bindTexture(0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, (const GLvoid *)coords);
		glDrawArrays(GL_POLYGON, 0, count/2-1); // opengl will close the polygon for us
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

love::image::ImageData *Graphics::newScreenshot(love::image::Image *image, bool copyAlpha)
{
	int w = getRenderWidth();
	int h = getRenderHeight();

	int row = 4*w;

	int size = row*h;

	GLubyte *pixels = 0;
	GLubyte *screenshot = 0;

	try
	{
		pixels = new GLubyte[size];
		screenshot = new GLubyte[size];
	}
	catch (std::exception &)
	{
		delete[] pixels;
		delete[] screenshot;
		throw love::Exception("Out of memory.");
	}

	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	if (!copyAlpha)
	{
		// Replace alpha values with full opacity.
		for (int i = 3; i < size; i += 4)
			pixels[i] = 255;
	}

	// OpenGL sucks and reads pixels from the lower-left. Let's fix that.

	GLubyte *src = pixels - row, *dst = screenshot + size;

	for (int i = 0; i < h; ++i)
		memcpy(dst-=row, src+=row, row);

	delete[] pixels;

	love::image::ImageData *img = 0;
	try
	{
		img = image->newImageData(w, h, (void *) screenshot);
	}
	catch (love::Exception &)
	{
		delete[] screenshot;
		throw;
	}

	delete[] screenshot;

	return img;
}

void Graphics::push()
{
	if (userMatrices == matrixLimit)
		throw Exception("Maximum stack depth reached. (More pushes than pops?)");
	glPushMatrix();
	++userMatrices;
}

void Graphics::pop()
{
	if (userMatrices < 1)
		throw Exception("Minimum stack depth reached. (More pops than pushes?)");
	glPopMatrix();
	--userMatrices;
}

void Graphics::rotate(float r)
{
	glRotatef(LOVE_TODEG(r), 0, 0, 1);
}

void Graphics::scale(float x, float y)
{
	glScalef(x, y, 1);
}

void Graphics::translate(float x, float y)
{
	glTranslatef(x, y, 0);
}

void Graphics::shear(float kx, float ky)
{
	Matrix t;
	t.setShear(kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());
}

void Graphics::origin()
{
	glLoadIdentity();
}

bool Graphics::hasFocus() const
{
	return currentWindow->hasFocus();
}

} // opengl
} // graphics
} // love
