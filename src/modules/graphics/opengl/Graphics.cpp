/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

// LOVE
#include "common/config.h"
#include "common/math.h"
#include "common/Vector.h"

#include "Graphics.h"
#include "window/sdl/Window.h"
#include "Polyline.h"

// C++
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>

// C
#include <cmath>

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
	, width(0)
	, height(0)
	, created(false)
	, activeStencil(false)
	, savedState()
	, displayedMinReqWarning(false)
{
	currentWindow = love::window::sdl::Window::createSingleton();

	int w, h;
	love::window::WindowSettings wsettings;

	currentWindow->getWindow(w, h, wsettings);

	if (currentWindow->isCreated())
		setMode(w, h, wsettings.sRGB);
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
	//get scissor status
	s.scissor = (glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE);
	//do we have scissor, if so, store the box
	if (s.scissor)
		s.scissorBox = gl.getScissor();

	for (int i = 0; i < 4; i++)
		s.colorMask[i] = colorMask[i];

	wireframe = isWireframe();

	return s;
}

void Graphics::restoreState(const DisplayState &s)
{
	setColor(s.color);
	setBackgroundColor(s.backgroundColor);
	setBlendMode(s.blendMode);
	setLineWidth(lineWidth);
	setLineStyle(s.lineStyle);
	setPointSize(s.pointSize);
	if (s.scissor)
		setScissor(s.scissorBox.x, s.scissorBox.y, s.scissorBox.w, s.scissorBox.h);
	else
		setScissor();
	setColorMask(s.colorMask[0], s.colorMask[1], s.colorMask[2], s.colorMask[3]);
	setWireframe(s.wireframe);
}

void Graphics::setViewportSize(int width, int height)
{
	this->width = width;
	this->height = height;

	if (!isCreated())
		return;

	// We want to affect the main screen, not any Canvas that's currently active
	// (not that any *should* be active when this is called.)
	Canvas *c = Canvas::current;
	Canvas::bindDefaultCanvas();

	// Set the viewport to top-left corner.
	gl.setViewport(OpenGL::Viewport(0, 0, width, height));

	// If a canvas was bound before this function was called, it needs to be
	// made aware of the new system viewport size.
	Canvas::systemViewport = gl.getViewport();

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up orthographic view (no depth)
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);

	// Restore the previously active Canvas.
	if (c != nullptr)
		c->startGrab(c->getAttachedCanvases());
}

bool Graphics::setMode(int width, int height, bool &sRGB)
{
	this->width = width;
	this->height = height;

	gl.initContext();

	// Does the system meet LOVE's minimum requirements for graphics?
	if (!(GLEE_VERSION_2_0 && Shader::isSupported() && Canvas::isSupported())
		&& !displayedMinReqWarning)
	{
		love::window::MessageBoxType type = love::window::MESSAGEBOX_ERROR;

		const char *title = "Minimum system requirements not met!";

		std::string message;
		message += "Detected OpenGL version: ";
		message += (const char *) glGetString(GL_VERSION);
		message += "\nRequired OpenGL version: 2.1."; // -ish
		message += "\nThe program may crash or have graphical issues.";

		::printf("%s\n%s\n", title, message.c_str());
		currentWindow->showMessageBox(type, title, message.c_str());

		// We should only show the message once, instead of after every setMode.
		displayedMinReqWarning = true;
	}

	// Okay, setup OpenGL.
	gl.setupContext();

	created = true;

	setViewportSize(width, height);

	// Make sure antialiasing works when set elsewhere
	glEnable(GL_MULTISAMPLE);

	// Enable blending
	glEnable(GL_BLEND);

	// Enable all color component writes.
	setColorMask(true, true, true, true);

	// Auto-generated mipmaps should be the best quality possible
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	// Enable textures
	glEnable(GL_TEXTURE_2D);
	gl.setTextureUnit(0);

	// Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set pixel row alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Reload all volatile objects.
	if (!Volatile::loadAll())
		std::cerr << "Could not reload all volatile objects." << std::endl;

	// Restore the display state.
	restoreState(savedState);
	pixel_size_stack.clear();
	pixel_size_stack.reserve(5);
	pixel_size_stack.push_back(1);

	// Get the maximum number of matrices
	// subtract a few to give the engine some room.
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &matrixLimit);
	matrixLimit -= 5;

	// Set whether drawing converts input from linear -> sRGB colorspace.
	if (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_sRGB || GLEE_EXT_framebuffer_sRGB)
	{
		if (sRGB)
			glEnable(GL_FRAMEBUFFER_SRGB);
		else
			glDisable(GL_FRAMEBUFFER_SRGB);
	}
	else
		sRGB = false;

	Canvas::screenHasSRGB = sRGB;

	bool enabledebug = false;

	if (GLEE_VERSION_3_0)
	{
		// Enable OpenGL's debug output if a debug context has been created.
		GLint flags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		enabledebug = (flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0;
	}

	setDebug(enabledebug);

	return true;
}

void Graphics::unSetMode()
{
	if (!isCreated())
		return;

	// Window re-creation may destroy the GL context, so we must save the state.
	savedState = saveState();

	// Unload all volatile objects. These must be reloaded after the display
	// mode change.
	Volatile::unloadAll();

	gl.deInitContext();

	created = false;
}

static void APIENTRY debugCB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*len*/, const GLchar *msg, GLvoid* /*usr*/)
{
	// Human-readable strings for the debug info.
	const char *sourceStr = OpenGL::debugSourceString(source);
	const char *typeStr = OpenGL::debugTypeString(type);
	const char *severityStr = OpenGL::debugSeverityString(severity);

	const char *fmt = "OpenGL: %s [source=%s, type=%s, severity=%s, id=%d]\n";
	printf(fmt, msg, sourceStr, typeStr, severityStr, id);
}

void Graphics::setDebug(bool enable)
{
	// Make sure debug output is supported. The AMD ext. is a bit different
	// so we don't make use of it, since AMD drivers now support KHR_debug.
	if (!(GLEE_VERSION_4_3 || GLEE_KHR_debug || GLEE_ARB_debug_output))
		return;

	// Ugly hack to reduce code duplication.
	if (GLEE_ARB_debug_output && !(GLEE_VERSION_4_3 || GLEE_KHR_debug))
	{
		glDebugMessageCallback = (GLEEPFNGLDEBUGMESSAGECALLBACKPROC) glDebugMessageCallbackARB;
		glDebugMessageControl = (GLEEPFNGLDEBUGMESSAGECONTROLPROC) glDebugMessageControlARB;
	}

	if (!enable)
	{
		// Disable the debug callback function.
		glDebugMessageCallback(nullptr, nullptr);

		// We can disable debug output entirely with KHR_debug.
		if (GLEE_VERSION_4_3 || GLEE_KHR_debug)
			glDisable(GL_DEBUG_OUTPUT);

		return;
	}

	// We don't want asynchronous debug output.
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback(debugCB, nullptr);

	// Initially, enable everything.
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);

	// Disable messages about deprecated OpenGL functionality.
	glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, 0, GL_FALSE);
	glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, 0, GL_FALSE);

	if (GLEE_VERSION_4_3 || GLEE_KHR_debug)
		glEnable(GL_DEBUG_OUTPUT);

	::printf("OpenGL debug output enabled (LOVE_GRAPHICS_DEBUG=1)\n");
}

void Graphics::reset()
{
	DisplayState s;
	discardStencil();
	Canvas::bindDefaultCanvas();
	Shader::detach();
	origin();
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

int Graphics::getWidth() const
{
	return width;
}

int Graphics::getHeight() const
{
	return height;
}

bool Graphics::isCreated() const
{
	return created;
}

void Graphics::setScissor(int x, int y, int width, int height)
{
	glEnable(GL_SCISSOR_TEST);
	// OpenGL's reversed y-coordinate is compensated for in OpenGL::setScissor.
	gl.setScissor(OpenGL::Viewport(x, y, width, height));
}

void Graphics::setScissor()
{
	glDisable(GL_SCISSOR_TEST);
}

bool Graphics::getScissor(int &x, int &y, int &width, int &height) const
{
	OpenGL::Viewport scissor = gl.getScissor();

	x = scissor.x;
	y = scissor.y;
	width = scissor.w;
	height = scissor.h;

	return glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
}

void Graphics::defineStencil()
{
	// Make sure the active canvas has a stencil buffer.
	if (Canvas::current)
		Canvas::current->checkCreateStencil();

	// Disable color writes but don't save the mask values.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	activeStencil = true;
}

void Graphics::useStencil(bool invert)
{
	glStencilFunc(GL_EQUAL, (GLint)(!invert), 1); // invert ? 0 : 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	setColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
}

void Graphics::discardStencil()
{
	if (!activeStencil)
		return;

	setColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
	glDisable(GL_STENCIL_TEST);
	activeStencil = false;
}

Image *Graphics::newImage(love::image::ImageData *data, const Image::Flags &flags)
{
	// Create the image.
	Image *image = new Image(data, flags);

	if (!isCreated())
		return image;

	bool success = false;
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
		return nullptr;
	}

	return image;
}

Image *Graphics::newImage(love::image::CompressedData *cdata, const Image::Flags &flags)
{
	// Create the image.
	Image *image = new Image(cdata, flags);

	if (!isCreated())
		return image;

	bool success = false;
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
		return nullptr;
	}

	return image;
}

Quad *Graphics::newQuad(Quad::Viewport v, float sw, float sh)
{
	return new Quad(v, sw, sh);
}

Font *Graphics::newFont(love::font::Rasterizer *r, const Texture::Filter &filter)
{
	return new Font(r, filter);
}

SpriteBatch *Graphics::newSpriteBatch(Texture *texture, int size, int usage)
{
	return new SpriteBatch(texture, size, usage);
}

ParticleSystem *Graphics::newParticleSystem(Texture *texture, int size)
{
	return new ParticleSystem(texture, size);
}

Canvas *Graphics::newCanvas(int width, int height, Canvas::Format format, int msaa)
{
	if (!Canvas::isFormatSupported(format))
	{
		const char *fstr = "rgba8";
		Canvas::getConstant(format, fstr);
		throw love::Exception("The %s canvas format is not supported by your OpenGL implementation.", fstr);
	}

	if (width > gl.getMaxTextureSize())
		throw Exception("Cannot create canvas: width of %d pixels is too large for this system.", width);
	else if (height > gl.getMaxTextureSize())
		throw Exception("Cannot create canvas: height of %d pixels is too large for this system.", height);

	while (GL_NO_ERROR != glGetError())
		/* clear opengl error flag */;

	Canvas *canvas = new Canvas(width, height, format, msaa);
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
	throw Exception("%s", error_string.str().c_str());
	return nullptr; // never reached
}

Shader *Graphics::newShader(const Shader::ShaderSources &sources)
{
	return new Shader(sources);
}

Mesh *Graphics::newMesh(const std::vector<Vertex> &vertices, Mesh::DrawMode mode)
{
	return new Mesh(vertices, mode);
}

Mesh *Graphics::newMesh(int vertexcount, Mesh::DrawMode mode)
{
	return new Mesh(vertexcount, mode);
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
	gl.setClearColor(c);
}

Color Graphics::getBackgroundColor() const
{
	return gl.getClearColor();
}

void Graphics::setFont(Font *font)
{
	Object::AutoRelease fontrelease(currentFont);

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
	OpenGL::BlendState state = {GL_ONE, GL_ONE, GL_ZERO, GL_ZERO, GL_FUNC_ADD};

	switch (mode)
	{
	case BLEND_ALPHA:
		state.srcRGB = GL_SRC_ALPHA;
		state.srcA = GL_ONE;
		state.dstRGB = state.dstA = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_MULTIPLY:
		state.srcRGB = state.srcA = GL_DST_COLOR;
		state.dstRGB = state.dstA = GL_ZERO;
		break;
	case BLEND_PREMULTIPLIED:
		state.srcRGB = state.srcA = GL_ONE;
		state.dstRGB = state.dstA = GL_ONE_MINUS_SRC_ALPHA;
		break;
	case BLEND_SUBTRACT:
		state.func = GL_FUNC_REVERSE_SUBTRACT;
	case BLEND_ADD:
		state.srcRGB = state.srcA = GL_SRC_ALPHA;
		state.dstRGB = state.dstA = GL_ONE;
		break;
	case BLEND_SCREEN:
		state.srcRGB = state.srcA = GL_ONE;
		state.dstRGB = state.dstA = GL_ONE_MINUS_SRC_COLOR;
		break;
	case BLEND_REPLACE:
	default:
		state.srcRGB = state.srcA = GL_ONE;
		state.dstRGB = state.dstA = GL_ZERO;
		break;
	}

	gl.setBlendState(state);
}

Graphics::BlendMode Graphics::getBlendMode() const
{
	OpenGL::BlendState state = gl.getBlendState();

	if (state.func == GL_FUNC_REVERSE_SUBTRACT)  // && src == GL_SRC_ALPHA && dst == GL_ONE
		return BLEND_SUBTRACT;
	// Everything else has equation == GL_FUNC_ADD.
	else if (state.srcRGB == state.srcA && state.dstRGB == state.dstA)
	{
		if (state.srcRGB == GL_SRC_ALPHA && state.dstRGB == GL_ONE)
			return BLEND_ADD;
		else if (state.srcRGB == GL_SRC_ALPHA && state.dstRGB == GL_ONE_MINUS_SRC_ALPHA)
			return BLEND_ALPHA; // alpha blend mode fallback for very old OpenGL versions.
		else if (state.srcRGB == GL_DST_COLOR && state.dstRGB == GL_ZERO)
			return BLEND_MULTIPLY;
		else if (state.srcRGB == GL_ONE && state.dstRGB == GL_ONE_MINUS_SRC_ALPHA)
			return BLEND_PREMULTIPLIED;
		else if (state.srcRGB == GL_ONE && state.dstRGB == GL_ONE_MINUS_SRC_COLOR)
			return BLEND_SCREEN;
		else if (state.srcRGB == GL_ONE && state.dstRGB == GL_ZERO)
			return BLEND_REPLACE;
	}
	else if (state.srcRGB == GL_SRC_ALPHA && state.srcA == GL_ONE &&
		state.dstRGB == GL_ONE_MINUS_SRC_ALPHA && state.dstA == GL_ONE_MINUS_SRC_ALPHA)
		return BLEND_ALPHA;

	throw Exception("Unknown blend mode");
}

void Graphics::setDefaultFilter(const Texture::Filter &f)
{
	Texture::setDefaultFilter(f);
}

const Texture::Filter &Graphics::getDefaultFilter() const
{
	return Texture::getDefaultFilter();
}

void Graphics::setDefaultMipmapFilter(Texture::FilterMode filter, float sharpness)
{
	Image::setDefaultMipmapFilter(filter);
	Image::setDefaultMipmapSharpness(sharpness);
}

void Graphics::getDefaultMipmapFilter(Texture::FilterMode *filter, float *sharpness) const
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

void Graphics::setLineJoin(Graphics::LineJoin join)
{
	lineJoin = join;
}

float Graphics::getLineWidth() const
{
	return lineWidth;
}

Graphics::LineStyle Graphics::getLineStyle() const
{
	return lineStyle;
}

Graphics::LineJoin Graphics::getLineJoin() const
{
	return lineJoin;
}

void Graphics::setPointSize(float size)
{
	glPointSize((GLfloat)size);
}

float Graphics::getPointSize() const
{
	GLfloat size;
	glGetFloatv(GL_POINT_SIZE, &size);
	return (float)size;
}

void Graphics::setWireframe(bool enable)
{
	wireframe = enable;
	glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
}

bool Graphics::isWireframe() const
{
	return wireframe;
}

void Graphics::print(const std::string &str, float x, float y , float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	if (currentFont != nullptr)
		currentFont->print(str, x, y, 0.0, angle, sx, sy, ox, oy, kx, ky);
}

void Graphics::printf(const std::string &str, float x, float y, float wrap, AlignMode align, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	if (currentFont == nullptr)
		return;

	if (wrap < 0.0f)
		throw love::Exception("Horizontal wrap limit cannot be negative.");

	using std::string;
	using std::vector;

	// wrappedlines indicates which lines were automatically wrapped. It's
	// guaranteed to have the same number of elements as lines_to_draw.
	vector<bool> wrappedlines;
	vector<string> lines_to_draw = currentFont->getWrap(str, wrap, 0, &wrappedlines);

	glPushMatrix();

	static Matrix t;
	t.setTransformation(ceilf(x), ceilf(y), angle, sx, sy, ox, oy, kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());

	x = y = 0.0f;

	try
	{
		// now for the actual printing
		vector<string>::const_iterator line_iter, line_end = lines_to_draw.end();
		float extra_spacing = 0.0f;
		int num_spaces = 0;
		int i = 0;

		for (line_iter = lines_to_draw.begin(); line_iter != line_end; ++line_iter)
		{
			float width = static_cast<float>(currentFont->getWidth(*line_iter));
			switch (align)
			{
			case ALIGN_RIGHT:
				currentFont->print(*line_iter, ceilf(x + (wrap - width)), ceilf(y), 0.0f);
				break;
			case ALIGN_CENTER:
				currentFont->print(*line_iter, ceilf(x + (wrap - width) / 2), ceilf(y), 0.0f);
				break;
			case ALIGN_JUSTIFY:
				num_spaces = std::count(line_iter->begin(), line_iter->end(), ' ');
				if (wrappedlines[i] && num_spaces >= 1)
					extra_spacing = (wrap - width) / float(num_spaces);
				else
					extra_spacing = 0.0f;
				currentFont->print(*line_iter, ceilf(x), ceilf(y), extra_spacing);
				break;
			case ALIGN_LEFT:
			default:
				currentFont->print(*line_iter, ceilf(x), ceilf(y), 0.0f);
				break;
			}
			y += currentFont->getHeight() * currentFont->getLineHeight();
			i++;
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
	gl.prepareDraw();
	gl.bindTexture(0);
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();
}

void Graphics::polyline(const float *coords, size_t count)
{
	if (lineJoin == LINE_JOIN_NONE)
	{
			NoneJoinPolyline line;
			line.render(coords, count, lineWidth * .5f, float(pixel_size_stack.back()), lineStyle == LINE_SMOOTH);
			line.draw();
	}
	else if (lineJoin == LINE_JOIN_BEVEL)
	{
		BevelJoinPolyline line;
		line.render(coords, count, lineWidth * .5f, float(pixel_size_stack.back()), lineStyle == LINE_SMOOTH);
		line.draw();
	}
	else // LINE_JOIN_MITER
	{
		MiterJoinPolyline line;
		line.render(coords, count, lineWidth * .5f, float(pixel_size_stack.back()), lineStyle == LINE_SMOOTH);
		line.draw();
	}
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
		gl.prepareDraw();
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
		gl.prepareDraw();
		gl.bindTexture(0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, (const GLvoid *)coords);
		glDrawArrays(GL_POLYGON, 0, count/2-1); // opengl will close the polygon for us
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

love::image::ImageData *Graphics::newScreenshot(love::image::Image *image, bool copyAlpha)
{
	// Temporarily unbind the currently active canvas (glReadPixels reads the
	// active framebuffer, not the main one.)
	Canvas *curcanvas = Canvas::current;
	if (curcanvas)
		Canvas::bindDefaultCanvas();

	int w = getWidth();
	int h = getHeight();

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

		if (curcanvas)
			curcanvas->startGrab(curcanvas->getAttachedCanvases());

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
		// Tell the new ImageData that it owns the screenshot data, so we don't
		// need to delete it here.
		img = image->newImageData(w, h, (void *) screenshot, true);
	}
	catch (love::Exception &)
	{
		delete[] screenshot;

		if (curcanvas)
			curcanvas->startGrab(curcanvas->getAttachedCanvases());

		throw;
	}

	// Re-bind the active canvas, if necessary.
	if (curcanvas)
		curcanvas->startGrab(curcanvas->getAttachedCanvases());

	return img;
}

Graphics::RendererInfo Graphics::getRendererInfo() const
{
	RendererInfo info;
	info.name = "OpenGL";

	const char *str = (const char *) glGetString(GL_VERSION);
	if (str)
		info.version = str;
	else
		throw love::Exception("Cannot retrieve renderer version information.");

	str = (const char *) glGetString(GL_VENDOR);
	if (str)
		info.vendor = str;
	else
		throw love::Exception("Cannot retrieve renderer vendor information.");

	str = (const char *) glGetString(GL_RENDERER);
	if (str)
		info.device = str;
	else
		throw love::Exception("Cannot retrieve renderer device information.");

	return info;
}

double Graphics::getSystemLimit(SystemLimit limittype) const
{
	double limit = 0.0;

	switch (limittype)
	{
	case Graphics::LIMIT_POINT_SIZE:
		{
			GLfloat limits[2];
			glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, limits);
			limit = limits[1];
		}
		break;
	case Graphics::LIMIT_TEXTURE_SIZE:
		limit = (double) gl.getMaxTextureSize();
		break;
	case Graphics::LIMIT_MULTI_CANVAS:
		limit = (double) gl.getMaxRenderTargets();
		break;
	case Graphics::LIMIT_CANVAS_MSAA:
		if (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_object
			|| GLEE_EXT_framebuffer_multisample)
		{
			GLint intlimit = 0;
			glGetIntegerv(GL_MAX_SAMPLES, &intlimit);
			limit = (double) intlimit;
		}
		break;
	default:
		break;
	}

	return limit;
}

bool Graphics::isSupported(Support feature) const
{
	switch (feature)
	{
	case SUPPORT_MULTI_CANVAS:
		return Canvas::isMultiCanvasSupported();
	case SUPPORT_SRGB:
		// sRGB support for the screen is guaranteed if it's supported as a
		// Canvas format.
		return Canvas::isFormatSupported(Canvas::FORMAT_SRGB);
	default:
		return false;
	}
}

void Graphics::push()
{
	if (userMatrices == matrixLimit)
		throw Exception("Maximum stack depth reached. (More pushes than pops?)");
	glPushMatrix();
	++userMatrices;
	pixel_size_stack.push_back(pixel_size_stack.back());
}

void Graphics::pop()
{
	if (userMatrices < 1)
		throw Exception("Minimum stack depth reached. (More pops than pushes?)");
	glPopMatrix();
	--userMatrices;
	pixel_size_stack.pop_back();
}

void Graphics::rotate(float r)
{
	glRotatef(LOVE_TODEG(r), 0, 0, 1);
}

void Graphics::scale(float x, float y)
{
	glScalef(x, y, 1);
	pixel_size_stack.back() *= 2. / (fabs(x) + fabs(y));
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
	pixel_size_stack.back() = 1;
}

} // opengl
} // graphics
} // love
