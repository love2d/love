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
#include "Shader.h"
#include "Graphics.h"
#include "math/MathModule.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"

// Needed for reflection information.
#include "libraries/glslang/glslang/Include/Types.h"
#include "libraries/glslang/glslang/MachineIndependent/localintermediate.h"

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
#if defined(GL_EXT_texture_array) && (!defined(GL_ES) || __VERSION__ > 100 || defined(GL_OES_gpu_shader5))
// Only used when !GLSLES1 to work around Ouya driver bug. But we still want it
// enabled for glslang validation when glsl 1-on-3 is used, so also enable it if
// OES_gpu_shader5 exists.
#define LOVE_EXT_TEXTURE_ARRAY_ENABLED
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
uniform LOVE_HIGHP_OR_MEDIUMP vec4 love_UniformsPerDraw[13];

// These are initialized in love_initializeBuiltinUniforms below. GLSL ES can't
// do it as an initializer.
LOVE_HIGHP_OR_MEDIUMP mat4 TransformMatrix;
LOVE_HIGHP_OR_MEDIUMP mat4 ProjectionMatrix;
LOVE_HIGHP_OR_MEDIUMP mat3 NormalMatrix;

LOVE_HIGHP_OR_MEDIUMP vec4 love_ScreenSize;
LOVE_HIGHP_OR_MEDIUMP vec4 ConstantColor;

LOVE_HIGHP_OR_MEDIUMP float CurrentDPIScale;

LOVE_HIGHP_OR_MEDIUMP float ConstantPointSize;

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

	CurrentDPIScale = love_UniformsPerDraw[8].w;
	ConstantPointSize = love_UniformsPerDraw[9].w;
	love_ScreenSize = love_UniformsPerDraw[11];
	ConstantColor = love_UniformsPerDraw[12];
}
)";

static const char global_functions[] = R"(
#ifdef GL_ES
	#if __VERSION__ >= 300 || defined(LOVE_EXT_TEXTURE_ARRAY_ENABLED)
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
	#if __VERSION__ >= 130 || defined(LOVE_EXT_TEXTURE_ARRAY_ENABLED)
		vec4 Texel(sampler2DArray s, vec3 c) { return love_texture2DArray(s, c); }
	#endif
	#ifdef PIXEL
		vec4 Texel(sampler2D s, vec2 c, float b) { return love_texture2D(s, c, b); }
		vec4 Texel(samplerCube s, vec3 c, float b) { return love_textureCube(s, c, b); }
		#if __VERSION__ > 100 || defined(GL_OES_texture_3D)
			vec4 Texel(sampler3D s, vec3 c, float b) { return love_texture3D(s, c, b); }
		#endif
		#if __VERSION__ >= 130 || defined(LOVE_EXT_TEXTURE_ARRAY_ENABLED)
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
#define love_PointSize gl_PointSize

#if __VERSION__ >= 130
	#define attribute in
	#define varying out
	#ifndef LOVE_GLSL1_ON_GLSL3
		#define love_VertexID gl_VertexID
		#define love_InstanceID gl_InstanceID
	#endif
#endif
)";

static const char vertex_functions[] = R"()";

static const char vertex_main[] = R"(
attribute vec4 VertexPosition;
attribute vec4 VertexTexCoord;
attribute vec4 VertexColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition);

void main() {
	love_initializeBuiltinUniforms();
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;
	love_Position = position(ClipSpaceFromLocal, VertexPosition);
}
)";

static const char vertex_main_raw[] = R"(
void vertexmain();

void main() {
	love_initializeBuiltinUniforms();
	vertexmain();
}
)";

static const char pixel_header[] = R"(
#ifdef GL_ES
	precision mediump float;
#endif

#define love_MaxRenderTargets gl_MaxDrawBuffers

#if __VERSION__ >= 130
	#define varying in
#endif

// Legacy
#define love_MaxCanvases love_MaxRenderTargets

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
#if __VERSION__ >= 130
	layout(location = 0) out vec4 love_PixelColor;
#else
	#define love_PixelColor gl_FragColor
#endif

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
#if __VERSION__ >= 130
	// Some drivers seem to make the pixel shader do more work when multiple
	// pixel shader outputs are defined, even when only one is actually used.
	// TODO: We should use reflection or something instead of this, to determine
	// how many outputs are actually used in the shader code.
	#ifdef LOVE_MULTI_RENDER_TARGETS
		layout(location = 0) out vec4 love_RenderTargets[love_MaxRenderTargets];
		#define love_PixelColor love_RenderTargets[0]
	#else
		layout(location = 0) out vec4 love_PixelColor;
	#endif
#else
	#ifdef LOVE_MULTI_RENDER_TARGETS
		#define love_RenderTargets gl_FragData
	#endif
	#define love_PixelColor gl_FragColor
#endif

// Legacy
#define love_Canvases love_RenderTargets
#ifdef LOVE_MULTI_RENDER_TARGETS
#define LOVE_MULTI_CANVASES 1
#endif

varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

void effect();

void main() {
	love_initializeBuiltinUniforms();
	effect();
}
)";

static const char pixel_main_raw[] = R"(
void pixelmain();

void main() {
	love_initializeBuiltinUniforms();
	pixelmain();
}
)";

struct StageInfo
{
	const char *name;
	const char *header;
	const char *functions;
	const char *main;
	const char *main_custom;
	const char *main_raw;
};

static const StageInfo stageInfo[] =
{
	{ "VERTEX", vertex_header, vertex_functions, vertex_main, vertex_main, vertex_main_raw },
	{ "PIXEL", pixel_header, pixel_functions, pixel_main, pixel_main_custom, pixel_main_raw },
};

static_assert((sizeof(stageInfo) / sizeof(StageInfo)) == SHADERSTAGE_MAX_ENUM, "Stages array size must match ShaderStage enum.");

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

static Shader::EntryPoint getVertexEntryPoint(const std::string &src)
{
	std::smatch m;

	if (std::regex_search(src, m, std::regex("void\\s+vertexmain\\s*\\(")))
		return Shader::ENTRYPOINT_RAW;

	if (std::regex_search(src, m, std::regex("vec4\\s+position\\s*\\(")))
		return Shader::ENTRYPOINT_HIGHLEVEL;

	return Shader::ENTRYPOINT_NONE;
}

static Shader::EntryPoint getPixelEntryPoint(const std::string &src, bool &mrt)
{
	mrt = false;
	std::smatch m;

	if (std::regex_search(src, m, std::regex("void\\s+pixelmain\\s*\\(")))
		return Shader::ENTRYPOINT_RAW;

	if (std::regex_search(src, m, std::regex("vec4\\s+effect\\s*\\(")))
		return Shader::ENTRYPOINT_HIGHLEVEL;

	if (std::regex_search(src, m, std::regex("void\\s+effect\\s*\\(")))
	{
		if (src.find("love_RenderTargets") != std::string::npos || src.find("love_Canvases") != std::string::npos)
			mrt = true;
		return Shader::ENTRYPOINT_CUSTOM;
	}

	return Shader::ENTRYPOINT_NONE;
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
	info.stages[SHADERSTAGE_VERTEX] = glsl::getVertexEntryPoint(src);
	info.stages[SHADERSTAGE_PIXEL] = glsl::getPixelEntryPoint(src, info.usesMRT);
	return info;
}

std::string Shader::createShaderStageCode(Graphics *gfx, ShaderStageType stage, const std::string &code, const Shader::SourceInfo &info)
{
	if (info.language == Shader::LANGUAGE_MAX_ENUM)
		throw love::Exception("Invalid shader language");

	if (info.stages[stage] == ENTRYPOINT_NONE)
		throw love::Exception("Cannot find entry point for shader stage.");

	if (info.stages[stage] == ENTRYPOINT_RAW && info.language == LANGUAGE_GLSL1)
		throw love::Exception("Shaders using a raw entry point (vertexmain or pixelmain) must use GLSL 3 or greater.");

	const auto &features = gfx->getCapabilities().features;

	if (info.language == LANGUAGE_GLSL3 && !features[Graphics::FEATURE_GLSL3])
		throw love::Exception("GLSL 3 shaders are not supported on this system.");

	if (info.language == LANGUAGE_GLSL4 && !features[Graphics::FEATURE_GLSL4])
		throw love::Exception("GLSL 4 shaders are not supported on this system.");

	bool gles = gfx->getRenderer() == Graphics::RENDERER_OPENGLES;
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
		ss << "#define LOVE_MULTI_RENDER_TARGETS 1";
	ss << glsl::global_syntax;
	ss << stageinfo.header;
	ss << glsl::global_uniforms;
	ss << glsl::global_functions;
	ss << stageinfo.functions;

	if (info.stages[stage] == ENTRYPOINT_HIGHLEVEL)
		ss << stageinfo.main;
	else if (info.stages[stage] == ENTRYPOINT_CUSTOM)
		ss << stageinfo.main_custom;
	else if (info.stages[stage] == ENTRYPOINT_RAW)
		ss << stageinfo.main_raw;
	else
		throw love::Exception("Unknown shader entry point %d", info.stages[stage]);
	ss << ((!gles && (lang == Shader::LANGUAGE_GLSL1 || glsl1on3)) ? "#line 0\n" : "#line 1\n");
	ss << code;

	return ss.str();
}

Shader::Shader(ShaderStage *vertex, ShaderStage *pixel)
	: stages()
{
	std::string err;
	if (!validateInternal(vertex, pixel, err, validationReflection))
		throw love::Exception("%s", err.c_str());

	stages[SHADERSTAGE_VERTEX] = vertex;
	stages[SHADERSTAGE_PIXEL] = pixel;
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

bool Shader::hasStage(ShaderStageType stage)
{
	return stages[stage] != nullptr;
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

void Shader::validateDrawState(PrimitiveType primtype, Texture *maintex) const
{
	if ((primtype == PRIMITIVE_POINTS) != validationReflection.usesPointSize)
	{
		if (validationReflection.usesPointSize)
			throw love::Exception("The active shader can only be used to draw points.");
		else
			throw love::Exception("The gl_PointSize variable must be set in a vertex shader when drawing points.");
	}

	if (maintex == nullptr)
		return;

	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);

	if (info == nullptr)
		return;

	if (!maintex->isReadable())
		throw love::Exception("Textures with non-readable formats cannot be sampled from in a shader.");

	auto textype = maintex->getTextureType();

	if (info->textureType != TEXTURE_MAX_ENUM && info->textureType != textype)
	{
		const char *textypestr = "unknown";
		const char *shadertextypestr = "unknown";
		Texture::getConstant(textype, textypestr);
		Texture::getConstant(info->textureType, shadertextypestr);
		throw love::Exception("Texture's type (%s) must match the type of the shader's main texture type (%s).", textypestr, shadertextypestr);
	}

	if (info->isDepthSampler != maintex->getSamplerState().depthSampleMode.hasValue)
	{
		if (info->isDepthSampler)
			throw love::Exception("Depth comparison samplers in shaders can only be used with depth textures which have depth comparison set.");
		else
			throw love::Exception("Depth textures which have depth comparison set can only be used with depth/shadow samplers in shaders.");
	}
}

bool Shader::validate(ShaderStage* vertex, ShaderStage* pixel, std::string& err)
{
	ValidationReflection reflection;
	return validateInternal(vertex, pixel, err, reflection);
}

bool Shader::validateInternal(ShaderStage *vertex, ShaderStage *pixel, std::string &err, ValidationReflection &reflection)
{
	glslang::TProgram program;

	if (vertex != nullptr)
		program.addShader(vertex->getGLSLangShader());

	if (pixel != nullptr)
		program.addShader(pixel->getGLSLangShader());

	if (!program.link(EShMsgDefault))
	{
		err = "Cannot compile shader:\n\n" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
		return false;
	}

	if (!program.buildReflection(EShReflectionSeparateBuffers))
	{
		err = "Cannot get reflection information for shader.";
		return false;
	}

	const auto *vertintermediate = program.getIntermediate(EShLangVertex);
	if (vertintermediate != nullptr)
	{
		// NOTE: this doesn't check whether the use affects final output...
		reflection.usesPointSize = vertintermediate->inIoAccessed("gl_PointSize");
	}

	for (int i = 0; i < program.getNumBufferBlocks(); i++)
	{
		const glslang::TObjectReflection &info = program.getBufferBlock(i);
		const glslang::TType *type = info.getType();
		if (type != nullptr)
		{
			const glslang::TQualifier &qualifiers = type->getQualifier();

			if ((!qualifiers.isReadOnly() || qualifiers.isWriteOnly()) && (info.stages & (EShLangVertexMask | EShLangFragmentMask)))
			{
				err = "Shader validation error:\nStorage Buffer block '" + info.name + "' must be marked as readonly in vertex and pixel shaders.";
				return false;
			}

			if (qualifiers.layoutPacking != glslang::ElpStd430)
			{
				err = "Shader validation error:\nStorage Buffer block '" + info.name + "' must use the std430 packing layout.";
				return false;
			}

			const glslang::TTypeList *structure = type->getStruct();
			if (structure == nullptr || structure->size() != 1)
			{
				err = "Shader validation error:\nStorage Buffer block '" + info.name + "' must contain a single unsized array of structs.";
				return false;
			}

			const glslang::TType* structtype = (*structure)[0].type;
			if (structtype == nullptr || structtype->getBasicType() != glslang::EbtStruct || !structtype->isUnsizedArray())
			{
				err = "Shader validation error:\nStorage Buffer block '" + info.name + "' must contain a single unsized array of structs.";
				return false;
			}

			BufferReflection bufferReflection = {};
			bufferReflection.stride = (size_t) info.size;
			bufferReflection.memberCount = (size_t) info.numMembers;

			reflection.storageBuffers[info.name] = bufferReflection;
		}
		else
		{
			err = "Shader validation error:\nCannot retrieve type information for Storage Buffer Block '" + info.name + "'.";
			return false;
		}
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

static const std::string defaultPointsVertex = R"(
vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition)
{
	love_PointSize = ConstantPointSize * CurrentDPIScale;
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

const std::string &Shader::getDefaultCode(StandardShader shader, ShaderStageType stage)
{
	if (stage == SHADERSTAGE_VERTEX)
	{
		if (shader == STANDARD_POINTS)
			return defaultPointsVertex;
		else
			return defaultVertex;
	}

	static std::string nocode = "";

	switch (shader)
	{
		case STANDARD_DEFAULT: return defaultStandardPixel;
		case STANDARD_VIDEO: return defaultVideoPixel;
		case STANDARD_ARRAY: return defaultArrayPixel;
		case STANDARD_POINTS: return defaultStandardPixel;
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
