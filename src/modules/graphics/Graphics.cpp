/**
 * Copyright (c) 2006-2021 LOVE Development Team
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
#include "Graphics.h"
#include "Buffer.h"
#include "math/MathModule.h"
#include "data/DataModule.h"
#include "Polyline.h"
#include "font/Font.h"
#include "window/Window.h"
#include "SpriteBatch.h"
#include "ParticleSystem.h"
#include "Font.h"
#include "Video.h"
#include "Text.h"
#include "common/deprecation.h"

// C++
#include <algorithm>
#include <stdlib.h>

namespace love
{
namespace graphics
{

static bool gammaCorrect = false;
static bool debugMode = false;
static bool debugModeQueried = false;

void setGammaCorrect(bool gammacorrect)
{
	gammaCorrect = gammacorrect;
}

bool isGammaCorrect()
{
	return gammaCorrect;
}

void gammaCorrectColor(Colorf &c)
{
	if (isGammaCorrect())
	{
		c.r = math::gammaToLinear(c.r);
		c.g = math::gammaToLinear(c.g);
		c.b = math::gammaToLinear(c.b);
	}
}

Colorf gammaCorrectColor(const Colorf &c)
{
	Colorf r = c;
	gammaCorrectColor(r);
	return r;
}

void unGammaCorrectColor(Colorf &c)
{
	if (isGammaCorrect())
	{
		c.r = math::linearToGamma(c.r);
		c.g = math::linearToGamma(c.g);
		c.b = math::linearToGamma(c.b);
	}
}

Colorf unGammaCorrectColor(const Colorf &c)
{
	Colorf r = c;
	unGammaCorrectColor(r);
	return r;
}

bool isDebugEnabled()
{
	if (!debugModeQueried)
	{
		const char *debugenv = getenv("LOVE_GRAPHICS_DEBUG");
		debugMode = debugenv != nullptr && debugenv[0] != '0';
		debugModeQueried = true;
	}

	return debugMode;
}

love::Type Graphics::type("graphics", &Module::type);

Graphics::DisplayState::DisplayState()
{
	defaultSamplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_LINEAR;
}

Graphics::Graphics()
	: width(0)
	, height(0)
	, pixelWidth(0)
	, pixelHeight(0)
	, created(false)
	, active(true)
	, writingToStencil(false)
	, batchedDrawState()
	, projectionMatrix()
	, renderTargetSwitchCount(0)
	, drawCalls(0)
	, drawCallsBatched(0)
	, quadIndexBuffer(nullptr)
	, capabilities()
	, cachedShaderStages()
{
	transformStack.reserve(16);
	transformStack.push_back(Matrix4());

	pixelScaleStack.reserve(16);
	pixelScaleStack.push_back(1);

	states.reserve(10);
	states.push_back(DisplayState());

	if (!Shader::initialize())
		throw love::Exception("Shader support failed to initialize!");
}

Graphics::~Graphics()
{
	delete quadIndexBuffer;

	// Clean up standard shaders before the active shader. If we do it after,
	// the active shader may try to activate a standard shader when deactivating
	// itself, which will cause problems since it calls Graphics methods in the
	// Graphics destructor.
	for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++)
	{
		if (Shader::standardShaders[i])
		{
			Shader::standardShaders[i]->release();
			Shader::standardShaders[i] = nullptr;
		}
	}

	states.clear();

	defaultFont.set(nullptr);

	delete batchedDrawState.vb[0];
	delete batchedDrawState.vb[1];
	delete batchedDrawState.indexBuffer;

	for (int i = 0; i < (int) SHADERSTAGE_MAX_ENUM; i++)
		cachedShaderStages[i].clear();

	Shader::deinitialize();
}

void Graphics::createQuadIndexBuffer()
{
	if (quadIndexBuffer != nullptr)
		return;

	size_t size = sizeof(uint16) * (LOVE_UINT16_MAX / 4) * 6;

	Buffer::Settings settings(BUFFERUSAGEFLAG_INDEX, BUFFERDATAUSAGE_STATIC);
	quadIndexBuffer = newBuffer(settings, DATAFORMAT_UINT16, nullptr, size, 0);

	Buffer::Mapper map(*quadIndexBuffer);
	fillIndices(TRIANGLEINDEX_QUADS, 0, LOVE_UINT16_MAX, (uint16 *) map.data);
}

Quad *Graphics::newQuad(Quad::Viewport v, double sw, double sh)
{
	return new Quad(v, sw, sh);
}

Font *Graphics::newFont(love::font::Rasterizer *data)
{
	return new Font(data, states.back().defaultSamplerState);
}

Font *Graphics::newDefaultFont(int size, font::TrueTypeRasterizer::Hinting hinting)
{
	auto fontmodule = Module::getInstance<font::Font>(M_FONT);
	if (!fontmodule)
		throw love::Exception("Font module has not been loaded.");

	StrongRef<font::Rasterizer> r(fontmodule->newTrueTypeRasterizer(size, hinting), Acquire::NORETAIN);
	return newFont(r.get());
}

Video *Graphics::newVideo(love::video::VideoStream *stream, float dpiscale)
{
	return new Video(this, stream, dpiscale);
}

love::graphics::SpriteBatch *Graphics::newSpriteBatch(Texture *texture, int size, BufferDataUsage usage)
{
	return new SpriteBatch(this, texture, size, usage);
}

love::graphics::ParticleSystem *Graphics::newParticleSystem(Texture *texture, int size)
{
	return new ParticleSystem(texture, size);
}

ShaderStage *Graphics::newShaderStage(ShaderStageType stage, const std::string &source, const Shader::SourceInfo &info)
{
	ShaderStage *s = nullptr;
	std::string cachekey;

	if (!source.empty())
	{
		data::HashFunction::Value hashvalue;
		data::hash(data::HashFunction::FUNCTION_SHA1, source.c_str(), source.size(), hashvalue);

		cachekey = std::string(hashvalue.data, hashvalue.size);

		auto it = cachedShaderStages[stage].find(cachekey);
		if (it != cachedShaderStages[stage].end())
		{
			s = it->second;
			s->retain();
		}
	}

	if (s == nullptr)
	{
		std::string glsl = Shader::createShaderStageCode(this, stage, source, info);
		s = newShaderStageInternal(stage, cachekey, glsl, getRenderer() == RENDERER_OPENGLES);
		if (!cachekey.empty())
			cachedShaderStages[stage][cachekey] = s;
	}

	return s;
}

Shader *Graphics::newShader(const std::vector<std::string> &stagessource)
{
	StrongRef<ShaderStage> stages[SHADERSTAGE_MAX_ENUM] = {};

	bool validstages[SHADERSTAGE_MAX_ENUM] = {};
	validstages[SHADERSTAGE_VERTEX] = true;
	validstages[SHADERSTAGE_PIXEL] = true;

	for (const std::string &source : stagessource)
	{
		Shader::SourceInfo info = Shader::getSourceInfo(source);
		bool isanystage = false;

		for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
		{
			if (!validstages[i])
				continue;

			if (info.stages[i] != Shader::ENTRYPOINT_NONE)
			{
				isanystage = true;
				stages[i].set(newShaderStage((ShaderStageType) i, source, info), Acquire::NORETAIN);
			}
		}

		if (!isanystage)
			throw love::Exception("Could not parse shader code (missing 'position' or 'effect' function?)");
	}

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		auto stype = (ShaderStageType) i;
		if (validstages[i] && stages[i].get() == nullptr)
		{
			const std::string &source = Shader::getDefaultCode(Shader::STANDARD_DEFAULT, stype);
			Shader::SourceInfo info = Shader::getSourceInfo(source);
			stages[i].set(newShaderStage(stype, source, info), Acquire::NORETAIN);
		}

	}

	return newShaderInternal(stages[SHADERSTAGE_VERTEX], stages[SHADERSTAGE_PIXEL]);
}

Buffer *Graphics::newBuffer(const Buffer::Settings &settings, DataFormat format, const void *data, size_t size, size_t arraylength)
{
	std::vector<Buffer::DataDeclaration> dataformat = {{"", format, 0}};
	return newBuffer(settings, dataformat, data, size, arraylength);
}

Mesh *Graphics::newMesh(const std::vector<Buffer::DataDeclaration> &vertexformat, int vertexcount, PrimitiveType drawmode, BufferDataUsage usage)
{
	return new Mesh(this, vertexformat, vertexcount, drawmode, usage);
}

Mesh *Graphics::newMesh(const std::vector<Buffer::DataDeclaration> &vertexformat, const void *data, size_t datasize, PrimitiveType drawmode, BufferDataUsage usage)
{
	return new Mesh(this, vertexformat, data, datasize, drawmode, usage);
}

Mesh *Graphics::newMesh(const std::vector<Mesh::BufferAttribute> &attributes, PrimitiveType drawmode)
{
	return new Mesh(attributes, drawmode);
}

love::graphics::Text *Graphics::newText(graphics::Font *font, const std::vector<Font::ColoredString> &text)
{
	return new Text(font, text);
}

void Graphics::cleanupCachedShaderStage(ShaderStageType type, const std::string &hashkey)
{
	cachedShaderStages[type].erase(hashkey);
}

bool Graphics::validateShader(bool gles, const std::vector<std::string> &stagessource, std::string &err)
{
	StrongRef<ShaderStage> stages[SHADERSTAGE_MAX_ENUM] = {};

	bool validstages[SHADERSTAGE_MAX_ENUM] = {};
	validstages[SHADERSTAGE_VERTEX] = true;
	validstages[SHADERSTAGE_PIXEL] = true;

	// Don't use cached shader stages, since the gles flag may not match the
	// current renderer.
	for (const std::string &source : stagessource)
	{
		Shader::SourceInfo info = Shader::getSourceInfo(source);
		bool isanystage = false;

		for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
		{
			auto stype = (ShaderStageType) i;

			if (!validstages[i])
				continue;

			if (info.stages[i] != Shader::ENTRYPOINT_NONE)
			{
				isanystage = true;
				std::string glsl = Shader::createShaderStageCode(this, stype, source, info);
				stages[i].set(new ShaderStageForValidation(this, stype, glsl, gles), Acquire::NORETAIN);
			}
		}

		if (!isanystage)
		{
			err = "Could not parse shader code (missing 'position' or 'effect' function?)";
			return false;
		}
	}

	return Shader::validate(stages[SHADERSTAGE_VERTEX], stages[SHADERSTAGE_PIXEL], err);
}

int Graphics::getWidth() const
{
	return width;
}

int Graphics::getHeight() const
{
	return height;
}

int Graphics::getPixelWidth() const
{
	return pixelWidth;
}

int Graphics::getPixelHeight() const
{
	return pixelHeight;
}

double Graphics::getCurrentDPIScale() const
{
	const auto &rt = states.back().renderTargets.getFirstTarget();
	if (rt.texture.get())
		return rt.texture->getDPIScale();

	return getScreenDPIScale();
}

double Graphics::getScreenDPIScale() const
{
	return (double) getPixelHeight() / (double) getHeight();
}

bool Graphics::isCreated() const
{
	return created;
}

bool Graphics::isActive() const
{
	// The graphics module is only completely 'active' if there's a window, a
	// context, and the active variable is set.
	auto window = getInstance<love::window::Window>(M_WINDOW);
	return active && isCreated() && window != nullptr && window->isOpen();
}

void Graphics::reset()
{
	DisplayState s;
	stopDrawToStencilBuffer();
	restoreState(s);
	origin();
}

/**
 * State functions.
 **/

void Graphics::restoreState(const DisplayState &s)
{
	setColor(s.color);
	setBackgroundColor(s.backgroundColor);

	setBlendState(s.blend);

	setLineWidth(s.lineWidth);
	setLineStyle(s.lineStyle);
	setLineJoin(s.lineJoin);

	setPointSize(s.pointSize);

	if (s.scissor)
		setScissor(s.scissorRect);
	else
		setScissor();

	setStencilTest(s.stencilCompare, s.stencilTestValue);
	setDepthMode(s.depthTest, s.depthWrite);

	setMeshCullMode(s.meshCullMode);
	setFrontFaceWinding(s.winding);

	setFont(s.font.get());
	setShader(s.shader.get());
	setRenderTargets(s.renderTargets);

	setColorMask(s.colorMask);
	setWireframe(s.wireframe);

	setDefaultSamplerState(s.defaultSamplerState);
}

void Graphics::restoreStateChecked(const DisplayState &s)
{
	const DisplayState &cur = states.back();

	if (s.color != cur.color)
		setColor(s.color);

	setBackgroundColor(s.backgroundColor);

	if (!(s.blend == cur.blend))
		setBlendState(s.blend);

	// These are just simple assignments.
	setLineWidth(s.lineWidth);
	setLineStyle(s.lineStyle);
	setLineJoin(s.lineJoin);

	if (s.pointSize != cur.pointSize)
		setPointSize(s.pointSize);

	if (s.scissor != cur.scissor || (s.scissor && !(s.scissorRect == cur.scissorRect)))
	{
		if (s.scissor)
			setScissor(s.scissorRect);
		else
			setScissor();
	}

	if (s.stencilCompare != cur.stencilCompare || s.stencilTestValue != cur.stencilTestValue)
		setStencilTest(s.stencilCompare, s.stencilTestValue);

	if (s.depthTest != cur.depthTest || s.depthWrite != cur.depthWrite)
		setDepthMode(s.depthTest, s.depthWrite);

	setMeshCullMode(s.meshCullMode);

	if (s.winding != cur.winding)
		setFrontFaceWinding(s.winding);

	setFont(s.font.get());
	setShader(s.shader.get());

	const auto &sRTs = s.renderTargets;
	const auto &curRTs = cur.renderTargets;

	bool rtschanged = sRTs.colors.size() != curRTs.colors.size();
	if (!rtschanged)
	{
		for (size_t i = 0; i < sRTs.colors.size() && i < curRTs.colors.size(); i++)
		{
			if (sRTs.colors[i] != curRTs.colors[i])
			{
				rtschanged = true;
				break;
			}
		}

		if (!rtschanged && sRTs.depthStencil != curRTs.depthStencil)
			rtschanged = true;

		if (sRTs.temporaryRTFlags != curRTs.temporaryRTFlags)
			rtschanged = true;
	}

	if (rtschanged)
		setRenderTargets(s.renderTargets);

	if (s.colorMask != cur.colorMask)
		setColorMask(s.colorMask);

	if (s.wireframe != cur.wireframe)
		setWireframe(s.wireframe);

	setDefaultSamplerState(s.defaultSamplerState);
}

Colorf Graphics::getColor() const
{
	return states.back().color;
}

void Graphics::setBackgroundColor(Colorf c)
{
	states.back().backgroundColor = c;
}

Colorf Graphics::getBackgroundColor() const
{
	return states.back().backgroundColor;
}

void Graphics::checkSetDefaultFont()
{
	// We don't create or set the default Font if an existing font is in use.
	if (states.back().font.get() != nullptr)
		return;

	// Create a new default font if we don't have one yet.
	if (!defaultFont.get())
		defaultFont.set(newDefaultFont(13, font::TrueTypeRasterizer::HINTING_NORMAL), Acquire::NORETAIN);

	states.back().font.set(defaultFont.get());
}

void Graphics::setFont(love::graphics::Font *font)
{
	// We don't need to set a default font here if null is passed in, since we
	// only care about the default font in getFont and print.
	DisplayState &state = states.back();
	state.font.set(font);
}

love::graphics::Font *Graphics::getFont()
{
	checkSetDefaultFont();
	return states.back().font.get();
}

void Graphics::setShader(love::graphics::Shader *shader)
{
	if (shader == nullptr)
		return setShader();

	shader->attach();
	states.back().shader.set(shader);
}

void Graphics::setShader()
{
	Shader::attachDefault(Shader::STANDARD_DEFAULT);
	states.back().shader.set(nullptr);
}

love::graphics::Shader *Graphics::getShader() const
{
	return states.back().shader.get();
}

void Graphics::setRenderTarget(RenderTarget rt, uint32 temporaryRTFlags)
{
	if (rt.texture == nullptr)
		return setRenderTarget();

	RenderTargets rts;
	rts.colors.push_back(rt);
	rts.temporaryRTFlags = temporaryRTFlags;

	setRenderTargets(rts);
}

void Graphics::setRenderTargets(const RenderTargetsStrongRef &rts)
{
	RenderTargets targets;
	targets.colors.reserve(rts.colors.size());

	for (const auto &rt : rts.colors)
		targets.colors.emplace_back(rt.texture.get(), rt.slice, rt.mipmap);

	targets.depthStencil = RenderTarget(rts.depthStencil.texture, rts.depthStencil.slice, rts.depthStencil.mipmap);
	targets.temporaryRTFlags = rts.temporaryRTFlags;

	return setRenderTargets(targets);
}

void Graphics::setRenderTargets(const RenderTargets &rts)
{
	DisplayState &state = states.back();
	int rtcount = (int) rts.colors.size();

	RenderTarget firsttarget = rts.getFirstTarget();
	Texture *firsttex = firsttarget.texture;

	if (firsttex == nullptr)
		return setRenderTarget();

	const auto &prevRTs = state.renderTargets;

	if (rtcount == (int) prevRTs.colors.size())
	{
		bool modified = false;

		for (int i = 0; i < rtcount; i++)
		{
			if (rts.colors[i] != prevRTs.colors[i])
			{
				modified = true;
				break;
			}
		}

		if (!modified && rts.depthStencil != prevRTs.depthStencil)
			modified = true;

		if (rts.temporaryRTFlags != prevRTs.temporaryRTFlags)
			modified = true;

		if (!modified)
			return;
	}

	if (rtcount > capabilities.limits[LIMIT_RENDER_TARGETS])
		throw love::Exception("This system can't simultaneously render to %d textures.", rtcount);

	bool multiformatsupported = capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS];

	PixelFormat firstcolorformat = PIXELFORMAT_UNKNOWN;
	if (!rts.colors.empty())
		firstcolorformat = rts.colors[0].texture->getPixelFormat();

	if (!firsttex->isRenderTarget())
		throw love::Exception("Texture must be created as a render target to be used in setRenderTargets.");

	if (isPixelFormatDepthStencil(firstcolorformat))
		throw love::Exception("Depth/stencil format textures must be used with the 'depthstencil' field of the table passed into setRenderTargets.");

	if (firsttarget.mipmap < 0 || firsttarget.mipmap >= firsttex->getMipmapCount())
		throw love::Exception("Invalid mipmap level %d.", firsttarget.mipmap + 1);

	if (!firsttex->isValidSlice(firsttarget.slice))
		throw love::Exception("Invalid slice index: %d.", firsttarget.slice + 1);

	bool hasSRGBtexture = firstcolorformat == PIXELFORMAT_sRGBA8_UNORM;
	int pixelw = firsttex->getPixelWidth(firsttarget.mipmap);
	int pixelh = firsttex->getPixelHeight(firsttarget.mipmap);
	int reqmsaa = firsttex->getRequestedMSAA();

	for (int i = 1; i < rtcount; i++)
	{
		Texture *c = rts.colors[i].texture;
		PixelFormat format = c->getPixelFormat();
		int mip = rts.colors[i].mipmap;
		int slice = rts.colors[i].slice;

		if (!c->isRenderTarget())
			throw love::Exception("Texture must be created as a render target to be used in setRenderTargets.");

		if (mip < 0 || mip >= c->getMipmapCount())
			throw love::Exception("Invalid mipmap level %d.", mip + 1);

		if (!c->isValidSlice(slice))
			throw love::Exception("Invalid slice index: %d.", slice + 1);

		if (c->getPixelWidth(mip) != pixelw || c->getPixelHeight(mip) != pixelh)
			throw love::Exception("All textures must have the same pixel dimensions.");

		if (!multiformatsupported && format != firstcolorformat)
			throw love::Exception("This system doesn't support multi-render-target rendering with different texture formats.");

		if (c->getRequestedMSAA() != reqmsaa)
			throw love::Exception("All textures must have the same MSAA value.");

		if (isPixelFormatDepthStencil(format))
			throw love::Exception("Depth/stencil format textures must be used with the 'depthstencil' field of the table passed into setRenderTargets.");

		if (format == PIXELFORMAT_sRGBA8_UNORM)
			hasSRGBtexture = true;
	}

	if (rts.depthStencil.texture != nullptr)
	{
		Texture *c = rts.depthStencil.texture;
		int mip = rts.depthStencil.mipmap;
		int slice = rts.depthStencil.slice;

		if (!c->isRenderTarget())
			throw love::Exception("Texture must be created as a render target to be used in setRenderTargets.");

		if (!isPixelFormatDepthStencil(c->getPixelFormat()))
			throw love::Exception("Only depth/stencil format textures can be used with the 'depthstencil' field of the table passed into setRenderTargets.");

		if (c->getPixelWidth(mip) != pixelw || c->getPixelHeight(mip) != pixelh)
			throw love::Exception("All Textures must have the same pixel dimensions.");

		if (c->getRequestedMSAA() != firsttex->getRequestedMSAA())
			throw love::Exception("All Textures must have the same MSAA value.");

		if (mip < 0 || mip >= c->getMipmapCount())
			throw love::Exception("Invalid mipmap level %d.", mip + 1);

		if (!c->isValidSlice(slice))
			throw love::Exception("Invalid slice index: %d.", slice + 1);
	}

	int w = firsttex->getWidth(firsttarget.mipmap);
	int h = firsttex->getHeight(firsttarget.mipmap);

	flushBatchedDraws();

	if (rts.depthStencil.texture == nullptr && rts.temporaryRTFlags != 0)
	{
		bool wantsdepth   = (rts.temporaryRTFlags & TEMPORARY_RT_DEPTH) != 0;
		bool wantsstencil = (rts.temporaryRTFlags & TEMPORARY_RT_STENCIL) != 0;

		PixelFormat dsformat = PIXELFORMAT_STENCIL8;
		if (wantsdepth && wantsstencil)
			dsformat = PIXELFORMAT_DEPTH24_UNORM_STENCIL8;
		else if (wantsdepth && isPixelFormatSupported(PIXELFORMAT_DEPTH24_UNORM, true, false, false))
			dsformat = PIXELFORMAT_DEPTH24_UNORM;
		else if (wantsdepth)
			dsformat = PIXELFORMAT_DEPTH16_UNORM;
		else if (wantsstencil)
			dsformat = PIXELFORMAT_STENCIL8;

		// We want setRenderTargetsInternal to have a pointer to the temporary RT,
		// but we don't want to directly store it in the main graphics state.
		RenderTargets realRTs = rts;

		realRTs.depthStencil.texture = getTemporaryTexture(dsformat, pixelw, pixelh, reqmsaa);
		realRTs.depthStencil.slice = 0;

		setRenderTargetsInternal(realRTs, w, h, pixelw, pixelh, hasSRGBtexture);
	}
	else
		setRenderTargetsInternal(rts, w, h, pixelw, pixelh, hasSRGBtexture);

	RenderTargetsStrongRef refs;
	refs.colors.reserve(rts.colors.size());

	for (auto c : rts.colors)
		refs.colors.emplace_back(c.texture, c.slice, c.mipmap);

	refs.depthStencil = RenderTargetStrongRef(rts.depthStencil.texture, rts.depthStencil.slice);
	refs.temporaryRTFlags = rts.temporaryRTFlags;

	std::swap(state.renderTargets, refs);

	renderTargetSwitchCount++;
}

void Graphics::setRenderTarget()
{
	DisplayState &state = states.back();

	if (state.renderTargets.colors.empty() && state.renderTargets.depthStencil.texture == nullptr)
		return;

	flushBatchedDraws();
	setRenderTargetsInternal(RenderTargets(), width, height, pixelWidth, pixelHeight, isGammaCorrect());

	state.renderTargets = RenderTargetsStrongRef();
	renderTargetSwitchCount++;
}

Graphics::RenderTargets Graphics::getRenderTargets() const
{
	const auto &curRTs = states.back().renderTargets;

	RenderTargets rts;
	rts.colors.reserve(curRTs.colors.size());

	for (const auto &rt : curRTs.colors)
		rts.colors.emplace_back(rt.texture.get(), rt.slice, rt.mipmap);

	rts.depthStencil = RenderTarget(curRTs.depthStencil.texture, curRTs.depthStencil.slice, curRTs.depthStencil.mipmap);
	rts.temporaryRTFlags = curRTs.temporaryRTFlags;

	return rts;
}

bool Graphics::isRenderTargetActive() const
{
	const auto &rts = states.back().renderTargets;
	return !rts.colors.empty() || rts.depthStencil.texture != nullptr;
}

bool Graphics::isRenderTargetActive(Texture *texture) const
{
	const auto &rts = states.back().renderTargets;

	for (const auto &rt : rts.colors)
	{
		if (rt.texture.get() == texture)
			return true;
	}

	if (rts.depthStencil.texture.get() == texture)
		return true;

	return false;
}

bool Graphics::isRenderTargetActive(Texture *texture, int slice) const
{
	const auto &rts = states.back().renderTargets;

	for (const auto &rt : rts.colors)
	{
		if (rt.texture.get() == texture && rt.slice == slice)
			return true;
	}

	if (rts.depthStencil.texture.get() == texture && rts.depthStencil.slice == slice)
		return true;

	return false;
}

Texture *Graphics::getTemporaryTexture(PixelFormat format, int w, int h, int samples)
{
	Texture *texture = nullptr;

	for (TemporaryTexture &temp : temporaryTextures)
	{
		Texture *c = temp.texture;
		if (c->getPixelFormat() == format && c->getPixelWidth() == w
			&& c->getPixelHeight() == h && c->getRequestedMSAA() == samples)
		{
			texture = c;
			temp.framesSinceUse = 0;
			break;
		}
	}

	if (texture == nullptr)
	{
		Texture::Settings settings;
		settings.renderTarget = true;
		settings.format = format;
		settings.width = w;
		settings.height = h;
		settings.msaa = samples;

		texture = newTexture(settings);

		temporaryTextures.emplace_back(texture);
	}

	return texture;
}

void Graphics::intersectScissor(const Rect &rect)
{
	Rect currect = states.back().scissorRect;

	if (!states.back().scissor)
	{
		currect.x = 0;
		currect.y = 0;
		currect.w = std::numeric_limits<int>::max();
		currect.h = std::numeric_limits<int>::max();
	}

	int x1 = std::max(currect.x, rect.x);
	int y1 = std::max(currect.y, rect.y);

	int x2 = std::min(currect.x + currect.w, rect.x + rect.w);
	int y2 = std::min(currect.y + currect.h, rect.y + rect.h);

	Rect newrect = {x1, y1, std::max(0, x2 - x1), std::max(0, y2 - y1)};
	setScissor(newrect);
}

bool Graphics::getScissor(Rect &rect) const
{
	const DisplayState &state = states.back();
	rect = state.scissorRect;
	return state.scissor;
}

void Graphics::setStencilTest()
{
	setStencilTest(COMPARE_ALWAYS, 0);
}

void Graphics::getStencilTest(CompareMode &compare, int &value) const
{
	const DisplayState &state = states.back();
	compare = state.stencilCompare;
	value = state.stencilTestValue;
}

void Graphics::setDepthMode()
{
	setDepthMode(COMPARE_ALWAYS, false);
}

void Graphics::getDepthMode(CompareMode &compare, bool &write) const
{
	const DisplayState &state = states.back();
	compare = state.depthTest;
	write = state.depthWrite;
}

void Graphics::setMeshCullMode(CullMode cull)
{
	// Handled inside the draw() graphics API implementations.
	states.back().meshCullMode = cull;
}

CullMode Graphics::getMeshCullMode() const
{
	return states.back().meshCullMode;
}

Winding Graphics::getFrontFaceWinding() const
{
	return states.back().winding;
}

ColorChannelMask Graphics::getColorMask() const
{
	return states.back().colorMask;
}

void Graphics::setBlendMode(BlendMode mode, BlendAlpha alphamode)
{
	if (alphamode == BLENDALPHA_MULTIPLY && !isAlphaMultiplyBlendSupported(mode))
	{
		const char *modestr = "unknown";
		love::graphics::getConstant(mode, modestr);
		throw love::Exception("The '%s' blend mode must be used with premultiplied alpha.", modestr);
	}

	setBlendState(computeBlendState(mode, alphamode));
}

BlendMode Graphics::getBlendMode(BlendAlpha &alphamode) const
{
	return computeBlendMode(states.back().blend, alphamode);
}

const BlendState &Graphics::getBlendState() const
{
	return states.back().blend;
}

void Graphics::setDefaultSamplerState(const SamplerState &s)
{
	states.back().defaultSamplerState = s;
}

const SamplerState &Graphics::getDefaultSamplerState() const
{
	return states.back().defaultSamplerState;
}

void Graphics::setLineWidth(float width)
{
	states.back().lineWidth = width;
}

void Graphics::setLineStyle(Graphics::LineStyle style)
{
	states.back().lineStyle = style;
}

void Graphics::setLineJoin(Graphics::LineJoin join)
{
	states.back().lineJoin = join;
}

float Graphics::getLineWidth() const
{
	return states.back().lineWidth;
}

Graphics::LineStyle Graphics::getLineStyle() const
{
	return states.back().lineStyle;
}

Graphics::LineJoin Graphics::getLineJoin() const
{
	return states.back().lineJoin;
}

float Graphics::getPointSize() const
{
	return states.back().pointSize;
}

bool Graphics::isWireframe() const
{
	return states.back().wireframe;
}

void Graphics::captureScreenshot(const ScreenshotInfo &info)
{
	pendingScreenshotCallbacks.push_back(info);
}

void Graphics::copyBuffer(Buffer *source, Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size)
{
	if (!capabilities.features[FEATURE_COPY_BUFFER])
		throw love::Exception("Buffer copying is not supported on this system.");

	if (!(source->getUsageFlags() & BUFFERUSAGEFLAG_COPY_SOURCE))
		throw love::Exception("Copy source buffer must be created with the copysource flag.");

	if (!(dest->getUsageFlags() & BUFFERUSAGEFLAG_COPY_DEST))
		throw love::Exception("Copy destination buffer must be created with the copydest flag.");

	Range sourcerange(sourceoffset, size);
	Range destrange(destoffset, size);

	if (sourcerange.getMax() >= source->getSize())
		throw love::Exception("Buffer copy source offset and size doesn't fit within the source Buffer's size.");

	if (destrange.getMax() >= dest->getSize())
		throw love::Exception("Buffer copy destination offset and size doesn't fit within the destination buffer's size.");

	if (source == dest && sourcerange.intersects(destrange))
		throw love::Exception("Copying a portion of a buffer to the same buffer requires non-overlapping source and destination offsets.");

	source->copyTo(dest, sourceoffset, destoffset, size);
}

Graphics::BatchedVertexData Graphics::requestBatchedDraw(const BatchedDrawCommand &cmd)
{
	BatchedDrawState &state = batchedDrawState;

	bool shouldflush = false;
	bool shouldresize = false;

	if (cmd.primitiveMode != state.primitiveMode
		|| cmd.formats[0] != state.formats[0] || cmd.formats[1] != state.formats[1]
		|| ((cmd.indexMode != TRIANGLEINDEX_NONE) != (state.indexCount > 0))
		|| cmd.texture != state.texture
		|| cmd.standardShaderType != state.standardShaderType)
	{
		shouldflush = true;
	}

	int totalvertices = state.vertexCount + cmd.vertexCount;

	// We only support uint16 index buffers for now.
	if (totalvertices > LOVE_UINT16_MAX && cmd.indexMode != TRIANGLEINDEX_NONE)
		shouldflush = true;

	int reqIndexCount = getIndexCount(cmd.indexMode, cmd.vertexCount);
	size_t reqIndexSize = reqIndexCount * sizeof(uint16);

	size_t newdatasizes[2] = {0, 0};
	size_t buffersizes[3] = {0, 0, 0};

	for (int i = 0; i < 2; i++)
	{
		if (cmd.formats[i] == CommonFormat::NONE)
			continue;

		size_t stride = getFormatStride(cmd.formats[i]);
		size_t datasize = stride * totalvertices;

		if (state.vbMap[i].data != nullptr && datasize > state.vbMap[i].size)
			shouldflush = true;

		if (datasize > state.vb[i]->getUsableSize())
		{
			buffersizes[i] = std::max(datasize, state.vb[i]->getSize() * 2);
			shouldresize = true;
		}

		newdatasizes[i] = stride * cmd.vertexCount;
	}

	if (cmd.indexMode != TRIANGLEINDEX_NONE)
	{
		size_t datasize = (state.indexCount + reqIndexCount) * sizeof(uint16);

		if (state.indexBufferMap.data != nullptr && datasize > state.indexBufferMap.size)
			shouldflush = true;

		if (datasize > state.indexBuffer->getUsableSize())
		{
			buffersizes[2] = std::max(datasize, state.indexBuffer->getSize() * 2);
			shouldresize = true;
		}
	}

	if (shouldflush || shouldresize)
	{
		flushBatchedDraws();

		state.primitiveMode = cmd.primitiveMode;
		state.formats[0] = cmd.formats[0];
		state.formats[1] = cmd.formats[1];
		state.texture = cmd.texture;
		state.standardShaderType = cmd.standardShaderType;
	}

	if (state.vertexCount == 0)
	{
		if (Shader::isDefaultActive())
			Shader::attachDefault(state.standardShaderType);

		if (Shader::current != nullptr)
			Shader::current->validateDrawState(cmd.primitiveMode, cmd.texture);
	}

	if (shouldresize)
	{
		for (int i = 0; i < 2; i++)
		{
			if (state.vb[i]->getSize() < buffersizes[i])
			{
				delete state.vb[i];
				state.vb[i] = newStreamBuffer(BUFFERUSAGE_VERTEX, buffersizes[i]);
			}
		}

		if (state.indexBuffer->getSize() < buffersizes[2])
		{
			delete state.indexBuffer;
			state.indexBuffer = newStreamBuffer(BUFFERUSAGE_INDEX, buffersizes[2]);
		}
	}

	if (cmd.indexMode != TRIANGLEINDEX_NONE)
	{
		if (state.indexBufferMap.data == nullptr)
			state.indexBufferMap = state.indexBuffer->map(reqIndexSize);

		uint16 *indices = (uint16 *) state.indexBufferMap.data;
		fillIndices(cmd.indexMode, state.vertexCount, cmd.vertexCount, indices);

		state.indexBufferMap.data += reqIndexSize;
	}

	BatchedVertexData d;

	for (int i = 0; i < 2; i++)
	{
		if (newdatasizes[i] > 0)
		{
			if (state.vbMap[i].data == nullptr)
				state.vbMap[i] = state.vb[i]->map(newdatasizes[i]);

			d.stream[i] = state.vbMap[i].data;

			state.vbMap[i].data += newdatasizes[i];
		}
	}

	if (state.vertexCount > 0)
		drawCallsBatched++;

	state.vertexCount += cmd.vertexCount;
	state.indexCount  += reqIndexCount;

	return d;
}

void Graphics::flushBatchedDraws()
{
	auto &sbstate = batchedDrawState;

	if (sbstate.vertexCount == 0 && sbstate.indexCount == 0)
		return;

	VertexAttributes attributes;
	BufferBindings buffers;

	size_t usedsizes[3] = {0, 0, 0};

	for (int i = 0; i < 2; i++)
	{
		if (sbstate.formats[i] == CommonFormat::NONE)
			continue;

		attributes.setCommonFormat(sbstate.formats[i], (uint8) i);

		usedsizes[i] = getFormatStride(sbstate.formats[i]) * sbstate.vertexCount;

		size_t offset = sbstate.vb[i]->unmap(usedsizes[i]);
		buffers.set(i, sbstate.vb[i], offset);
		sbstate.vbMap[i] = StreamBuffer::MapInfo();
	}

	if (attributes.enableBits == 0)
		return;

	Colorf nc = getColor();
	if (attributes.isEnabled(ATTRIB_COLOR))
		setColor(Colorf(1.0f, 1.0f, 1.0f, 1.0f));

	pushIdentityTransform();

	if (sbstate.indexCount > 0)
	{
		usedsizes[2] = sizeof(uint16) * sbstate.indexCount;

		DrawIndexedCommand cmd(&attributes, &buffers, sbstate.indexBuffer);
		cmd.primitiveType = sbstate.primitiveMode;
		cmd.indexCount = sbstate.indexCount;
		cmd.indexType = INDEX_UINT16;
		cmd.indexBufferOffset = sbstate.indexBuffer->unmap(usedsizes[2]);
		cmd.texture = sbstate.texture;
		draw(cmd);

		sbstate.indexBufferMap = StreamBuffer::MapInfo();
	}
	else
	{
		DrawCommand cmd(&attributes, &buffers);
		cmd.primitiveType = sbstate.primitiveMode;
		cmd.vertexStart = 0;
		cmd.vertexCount = sbstate.vertexCount;
		cmd.texture = sbstate.texture;
		draw(cmd);
	}

	for (int i = 0; i < 2; i++)
	{
		if (usedsizes[i] > 0)
			sbstate.vb[i]->markUsed(usedsizes[i]);
	}

	if (usedsizes[2] > 0)
		sbstate.indexBuffer->markUsed(usedsizes[2]);

	popTransform();

	if (attributes.isEnabled(ATTRIB_COLOR))
		setColor(nc);

	batchedDrawState.vertexCount = 0;
	batchedDrawState.indexCount = 0;
}

void Graphics::flushBatchedDrawsGlobal()
{
	Graphics *instance = getInstance<Graphics>(M_GRAPHICS);
	if (instance != nullptr)
		instance->flushBatchedDraws();
}

/**
 * Drawing
 **/

void Graphics::draw(Drawable *drawable, const Matrix4 &m)
{
	drawable->draw(this, m);
}

void Graphics::draw(Texture *texture, Quad *quad, const Matrix4 &m)
{
	texture->draw(this, quad, m);
}

void Graphics::drawLayer(Texture *texture, int layer, const Matrix4 &m)
{
	texture->drawLayer(this, layer, m);
}

void Graphics::drawLayer(Texture *texture, int layer, Quad *quad, const Matrix4 &m)
{
	texture->drawLayer(this, layer, quad, m);
}

void Graphics::drawInstanced(Mesh *mesh, const Matrix4 &m, int instancecount)
{
	mesh->drawInstanced(this, m, instancecount);
}

void Graphics::print(const std::vector<Font::ColoredString> &str, const Matrix4 &m)
{
	checkSetDefaultFont();

	if (states.back().font.get() != nullptr)
		print(str, states.back().font.get(), m);
}

void Graphics::print(const std::vector<Font::ColoredString> &str, Font *font, const Matrix4 &m)
{
	font->print(this, str, m, states.back().color);
}

void Graphics::printf(const std::vector<Font::ColoredString> &str, float wrap, Font::AlignMode align, const Matrix4 &m)
{
	checkSetDefaultFont();

	if (states.back().font.get() != nullptr)
		printf(str, states.back().font.get(), wrap, align, m);
}

void Graphics::printf(const std::vector<Font::ColoredString> &str, Font *font, float wrap, Font::AlignMode align, const Matrix4 &m)
{
	font->printf(this, str, wrap, align, m, states.back().color);
}

/**
 * Primitives (points, shapes, lines).
 **/

void Graphics::points(const Vector2 *positions, const Colorf *colors, size_t numpoints)
{
	const Matrix4 &t = getTransform();
	bool is2D = t.isAffine2DTransform();

	BatchedDrawCommand cmd;
	cmd.primitiveMode = PRIMITIVE_POINTS;
	cmd.formats[0] = getSinglePositionFormat(is2D);
	cmd.formats[1] = CommonFormat::RGBAub;
	cmd.vertexCount = (int) numpoints;
	cmd.standardShaderType = Shader::STANDARD_POINTS;

	BatchedVertexData data = requestBatchedDraw(cmd);

	if (is2D)
		t.transformXY((Vector2 *) data.stream[0], positions, cmd.vertexCount);
	else
		t.transformXY0((Vector3 *) data.stream[0], positions, cmd.vertexCount);

	Color32 *colordata = (Color32 *) data.stream[1];

	if (colors)
	{
		Colorf nc = getColor();
		gammaCorrectColor(nc);

		if (isGammaCorrect())
		{
			for (int i = 0; i < cmd.vertexCount; i++)
			{
				Colorf ci = colors[i];
				gammaCorrectColor(ci);
				ci *= nc;
				unGammaCorrectColor(ci);
				colordata[i] = toColor32(ci);
			}
		}
		else
		{
			for (int i = 0; i < cmd.vertexCount; i++)
				colordata[i] = toColor32(nc * colors[i]);
		}
	}
	else
	{
		Color32 c = toColor32(getColor());

		for (int i = 0; i < cmd.vertexCount; i++)
			colordata[i] = c;
	}
}

int Graphics::calculateEllipsePoints(float rx, float ry) const
{
	int points = (int) sqrtf(((rx + ry) / 2.0f) * 20.0f * (float) pixelScaleStack.back());
	return std::max(points, 8);
}

void Graphics::polyline(const Vector2 *vertices, size_t count)
{
	float halfwidth = getLineWidth() * 0.5f;
	LineJoin linejoin = getLineJoin();
	LineStyle linestyle = getLineStyle();

	float pixelsize = 1.0f / std::max((float) pixelScaleStack.back(), 0.000001f);

	if (linejoin == LINE_JOIN_NONE)
	{
		NoneJoinPolyline line;
		line.render(vertices, count, halfwidth, pixelsize, linestyle == LINE_SMOOTH);
		line.draw(this);
	}
	else if (linejoin == LINE_JOIN_BEVEL)
	{
		BevelJoinPolyline line;
		line.render(vertices, count, halfwidth, pixelsize, linestyle == LINE_SMOOTH);
		line.draw(this);
	}
	else if (linejoin == LINE_JOIN_MITER)
	{
		MiterJoinPolyline line;
		line.render(vertices, count, halfwidth, pixelsize, linestyle == LINE_SMOOTH);
		line.draw(this);
	}
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h)
{
	Vector2 coords[] = {Vector2(x,y), Vector2(x,y+h), Vector2(x+w,y+h), Vector2(x+w,y), Vector2(x,y)};
	polygon(mode, coords, 5);
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h, float rx, float ry, int points)
{
	if (rx == 0 || ry == 0)
	{
		rectangle(mode, x, y, w, h);
		return;
	}

	// Radius values that are more than half the rectangle's size aren't handled
	// correctly (for now)...
	if (w >= 0.02f)
		rx = std::min(rx, w / 2.0f - 0.01f);
	if (h >= 0.02f)
		ry = std::min(ry, h / 2.0f - 0.01f);

	points = std::max(points / 4, 1);

	const float half_pi = static_cast<float>(LOVE_M_PI / 2);
	float angle_shift = half_pi / ((float) points + 1.0f);

	int num_coords = (points + 2) * 4;
	Vector2 *coords = getScratchBuffer<Vector2>(num_coords + 1);
	float phi = .0f;

	for (int i = 0; i <= points + 2; ++i, phi += angle_shift)
	{
		coords[i].x = x + rx * (1 - cosf(phi));
		coords[i].y = y + ry * (1 - sinf(phi));
	}

	phi = half_pi;

	for (int i = points + 2; i <= 2 * (points + 2); ++i, phi += angle_shift)
	{
		coords[i].x = x + w - rx * (1 + cosf(phi));
		coords[i].y = y +     ry * (1 - sinf(phi));
	}

	phi = 2 * half_pi;

	for (int i = 2 * (points + 2); i <= 3 * (points + 2); ++i, phi += angle_shift)
	{
		coords[i].x = x + w - rx * (1 + cosf(phi));
		coords[i].y = y + h - ry * (1 + sinf(phi));
	}

	phi = 3 * half_pi;

	for (int i = 3 * (points + 2); i <= 4 * (points + 2); ++i, phi += angle_shift)
	{
		coords[i].x = x +     rx * (1 - cosf(phi));
		coords[i].y = y + h - ry * (1 + sinf(phi));
	}

	coords[num_coords] = coords[0];

	polygon(mode, coords, num_coords + 1);
}

void Graphics::rectangle(DrawMode mode, float x, float y, float w, float h, float rx, float ry)
{
	rectangle(mode, x, y, w, h, rx, ry, calculateEllipsePoints(rx, ry));
}

void Graphics::circle(DrawMode mode, float x, float y, float radius, int points)
{
	ellipse(mode, x, y, radius, radius, points);
}

void Graphics::circle(DrawMode mode, float x, float y, float radius)
{
	ellipse(mode, x, y, radius, radius);
}

void Graphics::ellipse(DrawMode mode, float x, float y, float a, float b, int points)
{
	float two_pi = (float) (LOVE_M_PI * 2);
	if (points <= 0) points = 1;
	float angle_shift = (two_pi / points);
	float phi = .0f;

	// 1 extra point at the end for a closed loop, and 1 extra point at the
	// start in filled mode for the vertex in the center of the ellipse.
	int extrapoints = 1 + (mode == DRAW_FILL ? 1 : 0);

	Vector2 *polygoncoords = getScratchBuffer<Vector2>(points + extrapoints);
	Vector2 *coords = polygoncoords;

	if (mode == DRAW_FILL)
	{
		coords[0].x = x;
		coords[0].y = y;
		coords++;
	}

	for (int i = 0; i < points; ++i, phi += angle_shift)
	{
		coords[i].x = x + a * cosf(phi);
		coords[i].y = y + b * sinf(phi);
	}

	coords[points] = coords[0];

	// Last argument to polygon(): don't skip the last vertex in fill mode.
	polygon(mode, polygoncoords, points + extrapoints, false);
}

void Graphics::ellipse(DrawMode mode, float x, float y, float a, float b)
{
	ellipse(mode, x, y, a, b, calculateEllipsePoints(a, b));
}

void Graphics::arc(DrawMode drawmode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2, int points)
{
	// Nothing to display with no points or equal angles. (Or is there with line mode?)
	if (points <= 0 || angle1 == angle2)
		return;

	// Oh, you want to draw a circle?
	if (fabs(angle1 - angle2) >= 2.0f * (float) LOVE_M_PI)
	{
		circle(drawmode, x, y, radius, points);
		return;
	}

	float angle_shift = (angle2 - angle1) / points;
	// Bail on precision issues.
	if (angle_shift == 0.0)
		return;

	// Prevent the connecting line from being drawn if a closed line arc has a
	// small angle. Avoids some visual issues when connected lines are at sharp
	// angles, due to the miter line join drawing code.
	if (drawmode == DRAW_LINE && arcmode == ARC_CLOSED && fabsf(angle1 - angle2) < LOVE_TORAD(4))
		arcmode = ARC_OPEN;

	// Quick fix for the last part of a filled open arc not being drawn (because
	// polygon(DRAW_FILL, ...) doesn't work without a closed loop of vertices.)
	if (drawmode == DRAW_FILL && arcmode == ARC_OPEN)
		arcmode = ARC_CLOSED;

	float phi = angle1;

	Vector2 *coords = nullptr;
	int num_coords = 0;

	const auto createPoints = [&](Vector2 *coordinates)
	{
		for (int i = 0; i <= points; ++i, phi += angle_shift)
		{
			coordinates[i].x = x + radius * cosf(phi);
			coordinates[i].y = y + radius * sinf(phi);
		}
	};

	if (arcmode == ARC_PIE)
	{
		num_coords = points + 3;
		coords = getScratchBuffer<Vector2>(num_coords);

		coords[0] = coords[num_coords - 1] = Vector2(x, y);

		createPoints(coords + 1);
	}
	else if (arcmode == ARC_OPEN)
	{
		num_coords = points + 1;
		coords = getScratchBuffer<Vector2>(num_coords);

		createPoints(coords);
	}
	else // ARC_CLOSED
	{
		num_coords = points + 2;
		coords = getScratchBuffer<Vector2>(num_coords);

		createPoints(coords);

		// Connect the ends of the arc.
		coords[num_coords - 1] = coords[0];
	}

	polygon(drawmode, coords, num_coords);
}

void Graphics::arc(DrawMode drawmode, ArcMode arcmode, float x, float y, float radius, float angle1, float angle2)
{
	float points = (float) calculateEllipsePoints(radius, radius);

	// The amount of points is based on the fraction of the circle created by the arc.
	float angle = fabsf(angle1 - angle2);
	if (angle < 2.0f * (float) LOVE_M_PI)
		points *= angle / (2.0f * (float) LOVE_M_PI);

	arc(drawmode, arcmode, x, y, radius, angle1, angle2, (int) (points + 0.5f));
}

void Graphics::polygon(DrawMode mode, const Vector2 *coords, size_t count, bool skipLastFilledVertex)
{
	// coords is an array of a closed loop of vertices, i.e.
	// coords[count-1] == coords[0]
	if (mode == DRAW_LINE)
	{
		polyline(coords, count);
	}
	else
	{
		const Matrix4 &t = getTransform();
		bool is2D = t.isAffine2DTransform();

		BatchedDrawCommand cmd;
		cmd.formats[0] = getSinglePositionFormat(is2D);
		cmd.formats[1] = CommonFormat::RGBAub;
		cmd.indexMode = TRIANGLEINDEX_FAN;
		cmd.vertexCount = (int)count - (skipLastFilledVertex ? 1 : 0);

		BatchedVertexData data = requestBatchedDraw(cmd);

		if (is2D)
			t.transformXY((Vector2 *) data.stream[0], coords, cmd.vertexCount);
		else
			t.transformXY0((Vector3 *) data.stream[0], coords, cmd.vertexCount);

		Color32 c = toColor32(getColor());
		Color32 *colordata = (Color32 *) data.stream[1];
		for (int i = 0; i < cmd.vertexCount; i++)
			colordata[i] = c;
	}
}

const Graphics::Capabilities &Graphics::getCapabilities() const
{
	return capabilities;
}

Graphics::Stats Graphics::getStats() const
{
	Stats stats;

	getAPIStats(stats.shaderSwitches);

	stats.drawCalls = drawCalls;
	if (batchedDrawState.vertexCount > 0)
		stats.drawCalls++;

	stats.renderTargetSwitches = renderTargetSwitchCount;
	stats.drawCallsBatched = drawCallsBatched;
	stats.textures = Texture::textureCount;
	stats.fonts = Font::fontCount;
	stats.textureMemory = Texture::totalGraphicsMemory;
	
	return stats;
}

size_t Graphics::getStackDepth() const
{
	return stackTypeStack.size();
}

void Graphics::push(StackType type)
{
	if (stackTypeStack.size() == MAX_USER_STACK_DEPTH)
		throw Exception("Maximum stack depth reached (more pushes than pops?)");

	pushTransform();

	pixelScaleStack.push_back(pixelScaleStack.back());

	if (type == STACK_ALL)
		states.push_back(states.back());

	stackTypeStack.push_back(type);
}

void Graphics::pop()
{
	if (stackTypeStack.size() < 1)
		throw Exception("Minimum stack depth reached (more pops than pushes?)");

	popTransform();
	pixelScaleStack.pop_back();

	if (stackTypeStack.back() == STACK_ALL)
	{
		DisplayState &newstate = states[states.size() - 2];

		restoreStateChecked(newstate);

		// The last two states in the stack should be equal now.
		states.pop_back();
	}

	stackTypeStack.pop_back();
}

/**
 * Transform and stack functions.
 **/

const Matrix4 &Graphics::getTransform() const
{
	return transformStack.back();
}

const Matrix4 &Graphics::getProjection() const
{
	return projectionMatrix;
}

void Graphics::pushTransform()
{
	transformStack.push_back(transformStack.back());
}

void Graphics::pushIdentityTransform()
{
	transformStack.push_back(Matrix4());
}

void Graphics::popTransform()
{
	transformStack.pop_back();
}

void Graphics::rotate(float r)
{
	transformStack.back().rotate(r);
}

void Graphics::scale(float x, float y)
{
	transformStack.back().scale(x, y);
	pixelScaleStack.back() *= (fabs(x) + fabs(y)) / 2.0;
}

void Graphics::translate(float x, float y)
{
	transformStack.back().translate(x, y);
}

void Graphics::shear(float kx, float ky)
{
	transformStack.back().shear(kx, ky);
}

void Graphics::origin()
{
	transformStack.back().setIdentity();
	pixelScaleStack.back() = 1;
}

void Graphics::applyTransform(love::math::Transform *transform)
{
	Matrix4 &m = transformStack.back();
	m *= transform->getMatrix();

	float sx, sy;
	m.getApproximateScale(sx, sy);
	pixelScaleStack.back() = (sx + sy) / 2.0;
}

void Graphics::replaceTransform(love::math::Transform *transform)
{
	const Matrix4 &m = transform->getMatrix();
	transformStack.back() = m;

	float sx, sy;
	m.getApproximateScale(sx, sy);
	pixelScaleStack.back() = (sx + sy) / 2.0;
}

Vector2 Graphics::transformPoint(Vector2 point)
{
	Vector2 p;
	transformStack.back().transformXY(&p, &point, 1);
	return p;
}

Vector2 Graphics::inverseTransformPoint(Vector2 point)
{
	Vector2 p;
	// TODO: We should probably cache the inverse transform so we don't have to
	// re-calculate it every time this is called.
	transformStack.back().inverse().transformXY(&p, &point, 1);
	return p;
}

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::DrawMode, Graphics::DRAW_MAX_ENUM, drawMode)
{
	{ "line", Graphics::DRAW_LINE },
	{ "fill", Graphics::DRAW_FILL },
}
STRINGMAP_CLASS_END(Graphics, Graphics::DrawMode, Graphics::DRAW_MAX_ENUM, drawMode)

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::ArcMode, Graphics::ARC_MAX_ENUM, arcMode)
{
	{ "open",   Graphics::ARC_OPEN   },
	{ "closed", Graphics::ARC_CLOSED },
	{ "pie",    Graphics::ARC_PIE    },
}
STRINGMAP_CLASS_END(Graphics, Graphics::ArcMode, Graphics::ARC_MAX_ENUM, arcMode)

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::LineStyle, Graphics::LINE_MAX_ENUM, lineStyle)
{
	{ "smooth", Graphics::LINE_SMOOTH },
	{ "rough",  Graphics::LINE_ROUGH  }
}
STRINGMAP_CLASS_END(Graphics, Graphics::LineStyle, Graphics::LINE_MAX_ENUM, lineStyle)

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM, lineJoin)
{
	{ "none",  Graphics::LINE_JOIN_NONE  },
	{ "miter", Graphics::LINE_JOIN_MITER },
	{ "bevel", Graphics::LINE_JOIN_BEVEL }
}
STRINGMAP_CLASS_END(Graphics, Graphics::LineJoin, Graphics::LINE_JOIN_MAX_ENUM, lineJoin)

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::Feature, Graphics::FEATURE_MAX_ENUM, feature)
{
	{ "multirendertargetformats", Graphics::FEATURE_MULTI_RENDER_TARGET_FORMATS },
	{ "clampzero",                Graphics::FEATURE_CLAMP_ZERO           },
	{ "blendminmax",              Graphics::FEATURE_BLEND_MINMAX         },
	{ "lighten",                  Graphics::FEATURE_LIGHTEN              },
	{ "fullnpot",                 Graphics::FEATURE_FULL_NPOT            },
	{ "pixelshaderhighp",         Graphics::FEATURE_PIXEL_SHADER_HIGHP   },
	{ "shaderderivatives",        Graphics::FEATURE_SHADER_DERIVATIVES   },
	{ "glsl3",                    Graphics::FEATURE_GLSL3                },
	{ "glsl4",                    Graphics::FEATURE_GLSL4                },
	{ "instancing",               Graphics::FEATURE_INSTANCING           },
	{ "texelbuffer",              Graphics::FEATURE_TEXEL_BUFFER         },
	{ "copybuffer",               Graphics::FEATURE_COPY_BUFFER          },
}
STRINGMAP_CLASS_END(Graphics, Graphics::Feature, Graphics::FEATURE_MAX_ENUM, feature)

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM, systemLimit)
{
	{ "pointsize",               Graphics::LIMIT_POINT_SIZE                 },
	{ "texturesize",             Graphics::LIMIT_TEXTURE_SIZE               },
	{ "texturelayers",           Graphics::LIMIT_TEXTURE_LAYERS             },
	{ "volumetexturesize",       Graphics::LIMIT_VOLUME_TEXTURE_SIZE        },
	{ "cubetexturesize",         Graphics::LIMIT_CUBE_TEXTURE_SIZE          },
	{ "texelbuffersize",         Graphics::LIMIT_TEXEL_BUFFER_SIZE          },
	{ "shaderstoragebuffersize", Graphics::LIMIT_SHADER_STORAGE_BUFFER_SIZE },
	{ "threadgroupsx",           Graphics::LIMIT_THREADGROUPS_X             },
	{ "threadgroupsy",           Graphics::LIMIT_THREADGROUPS_Y             },
	{ "threadgroupsz",           Graphics::LIMIT_THREADGROUPS_Z             },
	{ "rendertargets",           Graphics::LIMIT_RENDER_TARGETS             },
	{ "texturemsaa",             Graphics::LIMIT_TEXTURE_MSAA               },
	{ "anisotropy",              Graphics::LIMIT_ANISOTROPY                 },
}
STRINGMAP_CLASS_END(Graphics, Graphics::SystemLimit, Graphics::LIMIT_MAX_ENUM, systemLimit)

STRINGMAP_CLASS_BEGIN(Graphics, Graphics::StackType, Graphics::STACK_MAX_ENUM, stackType)
{
	{ "all",       Graphics::STACK_ALL       },
	{ "transform", Graphics::STACK_TRANSFORM },
}
STRINGMAP_CLASS_END(Graphics, Graphics::StackType, Graphics::STACK_MAX_ENUM, stackType)

} // graphics
} // love
