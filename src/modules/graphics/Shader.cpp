/**
 * Copyright (c) 2006-2020 LOVE Development Team
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
#include "Shader.h"
#include "Graphics.h"
#include "math/MathModule.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"

// C++
#include <string>
#include <regex>
#include <sstream>

namespace love
{
namespace graphics
{

namespace glsl
{
static const char global_syntax[] = R"(
#if !defined(GL_ES) && __VERSION__ < 140
	#define lowp
	#define mediump
	#define highp
#endif
#if defined(VERTEX) || __VERSION__ > 100 || defined(GL_FRAGMENT_PRECISION_HIGH)
	#define LOVE_HIGHP_OR_MEDIUMP highp
#else
	#define LOVE_HIGHP_OR_MEDIUMP mediump
#endif
#if __VERSION__ >= 300
#define LOVE_IO_LOCATION(x) layout (location = x)
#else
#define LOVE_IO_LOCATION(x)
#endif
#define number float
#define Image sampler2D
#define ArrayImage sampler2DArray
#define CubeImage samplerCube
#define VolumeImage sampler3D
#if __VERSION__ >= 300 && !defined(LOVE_GLSL1_ON_GLSL3)
	#define DepthImage sampler2DShadow
	#define DepthArrayImage sampler2DArrayShadow
	#define DepthCubeImage samplerCubeShadow
#endif
#define extern uniform
#ifdef GL_EXT_texture_array
#extension GL_EXT_texture_array : enable
#endif
#ifdef GL_OES_texture_3D
#extension GL_OES_texture_3D : enable
#endif
#ifdef GL_OES_standard_derivatives
#extension GL_OES_standard_derivatives : enable
#endif
)";

static const char global_uniforms[] = R"(
// According to the GLSL ES 1.0 spec, uniform precision must match between stages,
// but we can't guarantee that highp is always supported in fragment shaders...
// We *really* don't want to use mediump for these in vertex shaders though.
#if __VERSION__ >= 300 && defined(LOVE_USE_UNIFORM_BUFFERS)
layout (std140) uniform love_UniformsPerDrawBuffer {
	highp vec4 love_UniformsPerDraw[13];
};
#else
uniform LOVE_HIGHP_OR_MEDIUMP vec4 love_UniformsPerDraw[13];
#endif

// These are initialized in love_initializeBuiltinUniforms below. GLSL ES can't
// do it as an initializer.
LOVE_HIGHP_OR_MEDIUMP mat4 TransformMatrix;
LOVE_HIGHP_OR_MEDIUMP mat4 ProjectionMatrix;
LOVE_HIGHP_OR_MEDIUMP mat3 NormalMatrix;

LOVE_HIGHP_OR_MEDIUMP vec4 love_ScreenSize;
LOVE_HIGHP_OR_MEDIUMP vec4 ConstantColor;

#define TransformProjectionMatrix (ProjectionMatrix * TransformMatrix)

// Alternate names
#define ViewSpaceFromLocal TransformMatrix
#define ClipSpaceFromView ProjectionMatrix
#define ClipSpaceFromLocal TransformProjectionMatrix
#define ViewNormalFromLocal NormalMatrix

void love_initializeBuiltinUniforms() {
	TransformMatrix = mat4(
	   love_UniformsPerDraw[0],
	   love_UniformsPerDraw[1],
	   love_UniformsPerDraw[2],
	   love_UniformsPerDraw[3]
	);

	ProjectionMatrix = mat4(
	   love_UniformsPerDraw[4],
	   love_UniformsPerDraw[5],
	   love_UniformsPerDraw[6],
	   love_UniformsPerDraw[7]
	);

	NormalMatrix = mat3(
	   love_UniformsPerDraw[8].xyz,
	   love_UniformsPerDraw[9].xyz,
	   love_UniformsPerDraw[10].xyz
	);

	love_ScreenSize = love_UniformsPerDraw[11];
	ConstantColor = love_UniformsPerDraw[12];
}
)";

static const char global_functions[] = R"(
#ifdef GL_ES
	#if __VERSION__ >= 300 || defined(GL_EXT_texture_array)
		precision lowp sampler2DArray;
	#endif
	#if __VERSION__ >= 300 || defined(GL_OES_texture_3D)
		precision lowp sampler3D;
	#endif
	#if __VERSION__ >= 300
		precision lowp sampler2DShadow;
		precision lowp samplerCubeShadow;
		precision lowp sampler2DArrayShadow;
	#endif
#endif

#if __VERSION__ >= 130 && !defined(LOVE_GLSL1_ON_GLSL3)
	#define Texel texture
#else
	#if __VERSION__ >= 130
		#define texture2D Texel
		#define texture3D Texel
		#define textureCube Texel
		#define texture2DArray Texel
		#define love_texture2D texture
		#define love_texture3D texture
		#define love_textureCube texture
		#define love_texture2DArray texture
	#else
		#define love_texture2D texture2D
		#define love_texture3D texture3D
		#define love_textureCube textureCube
		#define love_texture2DArray texture2DArray
	#endif
	vec4 Texel(sampler2D s, vec2 c) { return love_texture2D(s, c); }
	vec4 Texel(samplerCube s, vec3 c) { return love_textureCube(s, c); }
	#if __VERSION__ > 100 || defined(GL_OES_texture_3D)
		vec4 Texel(sampler3D s, vec3 c) { return love_texture3D(s, c); }
	#endif
	#if __VERSION__ >= 130 || defined(GL_EXT_texture_array)
		vec4 Texel(sampler2DArray s, vec3 c) { return love_texture2DArray(s, c); }
	#endif
	#ifdef PIXEL
		vec4 Texel(sampler2D s, vec2 c, float b) { return love_texture2D(s, c, b); }
		vec4 Texel(samplerCube s, vec3 c, float b) { return love_textureCube(s, c, b); }
		#if __VERSION__ > 100 || defined(GL_OES_texture_3D)
			vec4 Texel(sampler3D s, vec3 c, float b) { return love_texture3D(s, c, b); }
		#endif
		#if __VERSION__ >= 130 || defined(GL_EXT_texture_array)
			vec4 Texel(sampler2DArray s, vec3 c, float b) { return love_texture2DArray(s, c, b); }
		#endif
	#endif
	#define texture love_texture
#endif

float gammaToLinearPrecise(float c) {
	return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}
vec3 gammaToLinearPrecise(vec3 c) {
	bvec3 leq = lessThanEqual(c, vec3(0.04045));
	c.r = leq.r ? c.r / 12.92 : pow((c.r + 0.055) / 1.055, 2.4);
	c.g = leq.g ? c.g / 12.92 : pow((c.g + 0.055) / 1.055, 2.4);
	c.b = leq.b ? c.b / 12.92 : pow((c.b + 0.055) / 1.055, 2.4);
	return c;
}
vec4 gammaToLinearPrecise(vec4 c) { return vec4(gammaToLinearPrecise(c.rgb), c.a); }
float linearToGammaPrecise(float c) {
	return c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1.0 / 2.4) - 0.055;
}
vec3 linearToGammaPrecise(vec3 c) {
	bvec3 lt = lessThanEqual(c, vec3(0.0031308));
	c.r = lt.r ? c.r * 12.92 : 1.055 * pow(c.r, 1.0 / 2.4) - 0.055;
	c.g = lt.g ? c.g * 12.92 : 1.055 * pow(c.g, 1.0 / 2.4) - 0.055;
	c.b = lt.b ? c.b * 12.92 : 1.055 * pow(c.b, 1.0 / 2.4) - 0.055;
	return c;
}
vec4 linearToGammaPrecise(vec4 c) { return vec4(linearToGammaPrecise(c.rgb), c.a); }

// http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1

mediump float gammaToLinearFast(mediump float c) { return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878); }
mediump vec3 gammaToLinearFast(mediump vec3 c) { return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878); }
mediump vec4 gammaToLinearFast(mediump vec4 c) { return vec4(gammaToLinearFast(c.rgb), c.a); }

mediump float linearToGammaFast(mediump float c) { return max(1.055 * pow(max(c, 0.0), 0.41666666) - 0.055, 0.0); }
mediump vec3 linearToGammaFast(mediump vec3 c) { return max(1.055 * pow(max(c, vec3(0.0)), vec3(0.41666666)) - 0.055, vec3(0.0)); }
mediump vec4 linearToGammaFast(mediump vec4 c) { return vec4(linearToGammaFast(c.rgb), c.a); }

#define gammaToLinear gammaToLinearFast
#define linearToGamma linearToGammaFast

#ifdef LOVE_GAMMA_CORRECT
	#define gammaCorrectColor gammaToLinear
	#define unGammaCorrectColor linearToGamma
	#define gammaCorrectColorPrecise gammaToLinearPrecise
	#define unGammaCorrectColorPrecise linearToGammaPrecise
	#define gammaCorrectColorFast gammaToLinearFast
	#define unGammaCorrectColorFast linearToGammaFast
#else
	#define gammaCorrectColor
	#define unGammaCorrectColor
	#define gammaCorrectColorPrecise
	#define unGammaCorrectColorPrecise
	#define gammaCorrectColorFast
	#define unGammaCorrectColorFast
#endif
)";

static const char vertex_header[] = R"(
#define love_Position gl_Position

#if __VERSION__ >= 130
	#define attribute in
	#define varying out
	#ifndef LOVE_GLSL1_ON_GLSL3
		#define love_VertexID gl_VertexID
		#define love_InstanceID gl_InstanceID
	#endif
#endif

#ifdef GL_ES
	uniform mediump float love_PointSize;
#endif
)";

static const char vertex_functions[] = R"(
void setPointSize() {
#ifdef GL_ES
	gl_PointSize = love_PointSize;
#endif
}
)";

static const char vertex_main[] = R"(
LOVE_IO_LOCATION(0) attribute vec4 VertexPosition;
LOVE_IO_LOCATION(1) attribute vec4 VertexTexCoord;
LOVE_IO_LOCATION(2) attribute vec4 VertexColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition);

void main() {
	love_initializeBuiltinUniforms();
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;
	setPointSize();
	love_Position = position(ClipSpaceFromLocal, VertexPosition);
}
)";

static const char pixel_header[] = R"(
#ifdef GL_ES
	precision mediump float;
#endif

#define love_MaxRenderTargets gl_MaxDrawBuffers

#if __VERSION__ >= 130
	#define varying in
	// Some drivers seem to make the pixel shader do more work when multiple
	// pixel shader outputs are defined, even when only one is actually used.
	// TODO: We should use reflection or something instead of this, to determine
	// how many outputs are actually used in the shader code.
	#ifdef LOVE_MULTI_RENDER_TARGETS
		LOVE_IO_LOCATION(0) out vec4 love_RenderTargets[love_MaxRenderTargets];
		#define love_PixelColor love_RenderTargets[0]
	#else
		LOVE_IO_LOCATION(0) out vec4 love_PixelColor;
	#endif
#else
	#ifdef LOVE_MULTI_RENDER_TARGETS
		#define love_RenderTargets gl_FragData
	#endif
	#define love_PixelColor gl_FragColor
#endif

// Legacy
#define love_MaxCanvases love_MaxRenderTargets
#define love_Canvases love_RenderTargets
#ifdef LOVE_MULTI_RENDER_TARGETS
#define LOVE_MULTI_CANVASES 1
#endif

// See Shader::updateScreenParams in Shader.cpp.
#define love_PixelCoord (vec2(gl_FragCoord.x, (gl_FragCoord.y * love_ScreenSize.z) + love_ScreenSize.w))
)";

static const char pixel_functions[] = R"(
uniform sampler2D love_VideoYChannel;
uniform sampler2D love_VideoCbChannel;
uniform sampler2D love_VideoCrChannel;

vec4 VideoTexel(vec2 texcoords) {
	vec3 yuv;
	yuv[0] = Texel(love_VideoYChannel, texcoords).r;
	yuv[1] = Texel(love_VideoCbChannel, texcoords).r;
	yuv[2] = Texel(love_VideoCrChannel, texcoords).r;
	yuv += vec3(-0.0627451017, -0.501960814, -0.501960814);

	vec4 color;
	color.r = dot(yuv, vec3(1.164,  0.000,  1.596));
	color.g = dot(yuv, vec3(1.164, -0.391, -0.813));
	color.b = dot(yuv, vec3(1.164,  2.018,  0.000));
	color.a = 1.0;

	return gammaCorrectColor(color);
}
)";

static const char pixel_main[] = R"(
uniform sampler2D MainTex;
varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

vec4 effect(vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord);

void main() {
	love_initializeBuiltinUniforms();
	love_PixelColor = effect(VaryingColor, MainTex, VaryingTexCoord.st, love_PixelCoord);
}
)";

static const char pixel_main_custom[] = R"(
varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

void effect();

void main() {
	love_initializeBuiltinUniforms();
	effect();
}
)";

struct StageInfo
{
	const char *name;
	const char *header;
	const char *functions;
	const char *main;
	const char *main_custom;
};

static const StageInfo stageInfo[] =
{
	{ "VERTEX", vertex_header, vertex_functions, vertex_main, vertex_main },
	{ "PIXEL", pixel_header, pixel_functions, pixel_main, pixel_main_custom },
};

static_assert((sizeof(stageInfo) / sizeof(StageInfo)) == ShaderStage::STAGE_MAX_ENUM, "Stages array size must match ShaderStage enum.");

struct Version
{
	std::string glsl;
	std::string glsles;
};

// Indexed by Shader::Version
static const Version versions[] =
{
	{ "#version 120", "#version 100" },
	{ "#version 330 core", "#version 300 es" },
	{ "#version 430 core", "#version 310 es" },
};

static Shader::Language getTargetLanguage(const std::string &src)
{
	std::regex r("^\\s*#pragma language (\\w+)");
	std::smatch m;
	std::string langstr = std::regex_search(src, m, r) && m.size() > 1 ? m[1] : std::string("glsl1");
	Shader::Language lang = Shader::LANGUAGE_MAX_ENUM;
	Shader::getConstant(langstr.c_str(), lang);
	return lang;
}

static bool isVertexCode(const std::string &src)
{
	std::regex r("vec4\\s+position\\s*\\(");
	std::smatch m;
	return std::regex_search(src, m, r);
}

static bool isPixelCode(const std::string &src, bool &custompixel, bool &mrt)
{
	custompixel = false;
	mrt = false;
	std::smatch m;
	if (std::regex_search(src, m, std::regex("vec4\\s+effect\\s*\\(")))
		return true;

	if (std::regex_search(src, m, std::regex("void\\s+effect\\s*\\(")))
	{
		custompixel = true;
		if (src.find("love_RenderTargets") != std::string::npos || src.find("love_Canvases") != std::string::npos)
			mrt = true;
		return true;
	}

	return false;
}

} // glsl

static_assert(sizeof(Shader::BuiltinUniformData) == sizeof(float) * 4 * 13, "Update the array in wrap_GraphicsShader.lua if this changes.");

love::Type Shader::type("Shader", &Object::type);

Shader *Shader::current = nullptr;
Shader *Shader::standardShaders[Shader::STANDARD_MAX_ENUM] = {nullptr};

Shader::SourceInfo Shader::getSourceInfo(const std::string &src)
{
	SourceInfo info = {};
	info.language = glsl::getTargetLanguage(src);
	info.isStage[ShaderStage::STAGE_VERTEX] = glsl::isVertexCode(src);
	info.isStage[ShaderStage::STAGE_PIXEL] = glsl::isPixelCode(src, info.customPixelFunction, info.usesMRT);
	return info;
}

std::string Shader::createShaderStageCode(Graphics *gfx, ShaderStage::StageType stage, const std::string &code, const Shader::SourceInfo &info)
{
	if (info.language == Shader::LANGUAGE_MAX_ENUM)
		throw love::Exception("Invalid shader language");

	const auto &features = gfx->getCapabilities().features;

	if (info.language == LANGUAGE_GLSL3 && !features[Graphics::FEATURE_GLSL3])
		throw love::Exception("GLSL 3 shaders are not supported on this system.");

	if (info.language == LANGUAGE_GLSL4 && !features[Graphics::FEATURE_GLSL4])
		throw love::Exception("GLSL 4 shaders are not supported on this system.");

	bool gles = gfx->usesGLSLES();
	bool glsl1on3 = info.language == LANGUAGE_GLSL1 && features[Graphics::FEATURE_GLSL3];

	Language lang = info.language;
	if (glsl1on3)
		lang = LANGUAGE_GLSL3;

	glsl::StageInfo stageinfo = glsl::stageInfo[stage];

	std::stringstream ss;

	ss << (gles ? glsl::versions[lang].glsles : glsl::versions[lang].glsl) << "\n";
	ss << "#define " << stageinfo.name << " " << stageinfo.name << "\n";
	if (glsl1on3)
		ss << "#define LOVE_GLSL1_ON_GLSL3 1\n";
	if (isGammaCorrect())
		ss << "#define LOVE_GAMMA_CORRECT 1\n";
	if (info.usesMRT)
		ss << "#define LOVE_MULTI_RENDER_TARGETS 1\n";
	if (gfx->getRenderer() == Graphics::RENDERER_METAL)
		ss << "#define LOVE_USE_UNIFORM_BUFFERS 1\n"; // FIXME: this is temporary
	ss << glsl::global_syntax;
	ss << stageinfo.header;
	ss << glsl::global_uniforms;
	ss << glsl::global_functions;
	ss << stageinfo.functions;
	ss << (info.customPixelFunction ? stageinfo.main_custom : stageinfo.main);
	ss << ((!gles && (lang == Shader::LANGUAGE_GLSL1 || glsl1on3)) ? "#line 0\n" : "#line 1\n");
	ss << code;

	return ss.str();
}

Shader::Shader(ShaderStage *vertex, ShaderStage *pixel)
	: stages()
{
	std::string err;
	if (!validate(vertex, pixel, err))
		throw love::Exception("%s", err.c_str());

	stages[ShaderStage::STAGE_VERTEX] = vertex;
	stages[ShaderStage::STAGE_PIXEL] = pixel;
}

Shader::~Shader()
{
	for (int i = 0; i < STANDARD_MAX_ENUM; i++)
	{
		if (this == standardShaders[i])
			standardShaders[i] = nullptr;
	}

	if (current == this)
		attachDefault(STANDARD_DEFAULT);
}

void Shader::attachDefault(StandardShader defaultType)
{
	Shader *defaultshader = standardShaders[defaultType];

	if (defaultshader == nullptr)
	{
		current = nullptr;
		return;
	}

	if (current != defaultshader)
		defaultshader->attach();
}

bool Shader::isDefaultActive()
{
	for (int i = 0; i < STANDARD_MAX_ENUM; i++)
	{
		if (current == standardShaders[i])
			return true;
	}

	return false;
}

TextureType Shader::getMainTextureType() const
{
	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);
	return info != nullptr ? info->textureType : TEXTURE_MAX_ENUM;
}

void Shader::checkMainTextureType(TextureType textype, bool isDepthSampler) const
{
	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);

	if (info == nullptr)
		return;

	if (info->textureType != TEXTURE_MAX_ENUM && info->textureType != textype)
	{
		const char *textypestr = "unknown";
		const char *shadertextypestr = "unknown";
		Texture::getConstant(textype, textypestr);
		Texture::getConstant(info->textureType, shadertextypestr);
		throw love::Exception("Texture's type (%s) must match the type of the shader's main texture type (%s).", textypestr, shadertextypestr);
	}

	if (info->isDepthSampler != isDepthSampler)
	{
		if (info->isDepthSampler)
			throw love::Exception("Depth comparison samplers in shaders can only be used with depth textures which have depth comparison set.");
		else
			throw love::Exception("Depth textures which have depth comparison set can only be used with depth/shadow samplers in shaders.");
	}
}

void Shader::checkMainTexture(Texture *tex) const
{
	if (!tex->isReadable())
		throw love::Exception("Textures with non-readable formats cannot be sampled from in a shader.");

	checkMainTextureType(tex->getTextureType(), tex->getSamplerState().depthSampleMode.hasValue);
}

bool Shader::validate(ShaderStage *vertex, ShaderStage *pixel, std::string &err)
{
	glslang::TProgram program;

	if (vertex != nullptr)
		program.addShader(vertex->getGLSLangValidationShader());

	if (pixel != nullptr)
		program.addShader(pixel->getGLSLangValidationShader());

	if (!program.link(EShMsgDefault))
	{
		err = "Cannot compile shader:\n\n" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
		return false;
	}

	return true;
}

bool Shader::initialize()
{
	return glslang::InitializeProcess();
}

void Shader::deinitialize()
{
	glslang::FinalizeProcess();
}

static const std::string defaultVertex = R"(
vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition)
{
	return clipSpaceFromLocal * localPosition;
}
)";

static const std::string defaultStandardPixel = R"(
vec4 effect(vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord)
{
	return Texel(tex, texcoord) * vcolor;
}
)";

static const std::string defaultVideoPixel = R"(
void effect()
{
	love_PixelColor = VideoTexel(VaryingTexCoord.xy) * VaryingColor;
}
)";

static const std::string defaultArrayPixel = R"(
uniform ArrayImage MainTex;
void effect()
{
	love_PixelColor = Texel(MainTex, VaryingTexCoord.xyz) * VaryingColor;
}
)";

const std::string &Shader::getDefaultCode(StandardShader shader, ShaderStage::StageType stage)
{
	if (stage == ShaderStage::STAGE_VERTEX)
		return defaultVertex;

	static std::string nocode = "";

	switch (shader)
	{
		case STANDARD_DEFAULT: return defaultStandardPixel;
		case STANDARD_VIDEO: return defaultVideoPixel;
		case STANDARD_ARRAY: return defaultArrayPixel;
		case STANDARD_MAX_ENUM: return nocode;
	}

	return nocode;
}

static StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM>::Entry languageEntries[] =
{
	{ "glsl1", Shader::LANGUAGE_GLSL1 },
	{ "glsl3", Shader::LANGUAGE_GLSL3 },
	{ "glsl4", Shader::LANGUAGE_GLSL4 },
};

static StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM> languages(languageEntries, sizeof(languageEntries));

static StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry builtinNameEntries[] =
{
	{ "MainTex",              Shader::BUILTIN_TEXTURE_MAIN      },
	{ "love_VideoYChannel",   Shader::BUILTIN_TEXTURE_VIDEO_Y   },
	{ "love_VideoCbChannel",  Shader::BUILTIN_TEXTURE_VIDEO_CB  },
	{ "love_VideoCrChannel",  Shader::BUILTIN_TEXTURE_VIDEO_CR  },
	{ "love_UniformsPerDraw", Shader::BUILTIN_UNIFORMS_PER_DRAW },
	{ "love_PointSize",       Shader::BUILTIN_POINT_SIZE        },
};

static StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM> builtinNames(builtinNameEntries, sizeof(builtinNameEntries));

bool Shader::getConstant(const char *in, Language &out)
{
	return languages.find(in, out);
}

bool Shader::getConstant(Language in, const char *&out)
{
	return languages.find(in, out);
}

bool Shader::getConstant(const char *in, BuiltinUniform &out)
{
	return builtinNames.find(in, out);
}

bool Shader::getConstant(BuiltinUniform in, const char *&out)
{
	return builtinNames.find(in, out);
}

} // graphics
} // love
