/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "common/Range.h"

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

static const char render_uniforms[] = R"(
// According to the GLSL ES 1.0 spec, uniform precision must match between stages,
// but we can't guarantee that highp is always supported in fragment shaders...
// We *really* don't want to use mediump for these in vertex shaders though.
#ifdef LOVE_SPLIT_UNIFORMS_PER_DRAW
uniform LOVE_HIGHP_OR_MEDIUMP vec4 love_UniformsPerDraw[12];
uniform LOVE_HIGHP_OR_MEDIUMP vec4 love_UniformsPerDraw2[1];
#else
uniform LOVE_HIGHP_OR_MEDIUMP vec4 love_UniformsPerDraw[13];
#endif

// Older GLSL doesn't support preprocessor line continuations...
#define TransformMatrix mat4(love_UniformsPerDraw[0], love_UniformsPerDraw[1], love_UniformsPerDraw[2], love_UniformsPerDraw[3])
#define ProjectionMatrix mat4(love_UniformsPerDraw[4], love_UniformsPerDraw[5], love_UniformsPerDraw[6], love_UniformsPerDraw[7])
#define TransformProjectionMatrix (ProjectionMatrix * TransformMatrix)

#define NormalMatrix mat3(love_UniformsPerDraw[8].xyz, love_UniformsPerDraw[9].xyz, love_UniformsPerDraw[10].xyz)

#define CurrentDPIScale (love_UniformsPerDraw[8].w)
#define ConstantPointSize (love_UniformsPerDraw[9].w)
#define ConstantColor (love_UniformsPerDraw[11])

#ifdef LOVE_SPLIT_UNIFORMS_PER_DRAW
#define love_ScreenSize (love_UniformsPerDraw2[0])
#else
#define love_ScreenSize (love_UniformsPerDraw[12])
#endif

// Alternate names
#define ViewSpaceFromLocal TransformMatrix
#define ClipSpaceFromView ProjectionMatrix
#define ClipSpaceFromLocal TransformProjectionMatrix
#define ViewNormalFromLocal NormalMatrix
)";

static const char global_functions[] = R"(
#ifdef GL_ES
	#if __VERSION__ >= 300 || defined(LOVE_EXT_TEXTURE_ARRAY_ENABLED)
		precision lowp sampler2DArray;
	#endif
	#if __VERSION__ >= 300 || defined(GL_OES_texture_3D)
		precision lowp sampler3D;
	#endif
	#if __VERSION__ >= 300 && !defined(LOVE_GLSL1_ON_GLSL3)
		precision lowp sampler2DShadow;
		precision lowp samplerCubeShadow;
		precision lowp sampler2DArrayShadow;
	#endif
#endif

#if __VERSION__ >= 430 || (defined(GL_ES) && __VERSION__ >= 310)
	layout (std430) buffer;
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
LOVE_IO_LOCATION(0) attribute vec4 VertexPosition;
LOVE_IO_LOCATION(1) attribute vec4 VertexTexCoord;
LOVE_IO_LOCATION(2) attribute vec4 VertexColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition);

void main() {
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;
	love_Position = position(ClipSpaceFromLocal, VertexPosition);
}
)";

static const char vertex_main_raw[] = R"(
void vertexmain();

void main() {
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
	LOVE_IO_LOCATION(0) out vec4 love_PixelColor;
#else
	#define love_PixelColor gl_FragColor
#endif

uniform sampler2D MainTex;
varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

vec4 effect(vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord);

void main() {
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
#define love_Canvases love_RenderTargets
#ifdef LOVE_MULTI_RENDER_TARGETS
#define LOVE_MULTI_CANVASES 1
#endif

varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

void effect();

void main() {
	effect();
}
)";

static const char pixel_main_raw[] = R"(
void pixelmain();

void main() {
	pixelmain();
}
)";

static const char compute_header[] = R"(
#define love_ThreadGroupCount gl_NumWorkGroups
#define love_ThreadGroupID gl_WorkGroupID
#define love_LocalThreadID gl_LocalInvocationID
#define love_GlobalThreadID gl_GlobalInvocationID
#define love_LocalThreadIndex gl_LocalInvocationIndex
#define love_ThreadGroupSize gl_WorkGroupSize
)";

static const char compute_uniforms[] = R"(
)";

static const char compute_functions[] = R"()";

static const char compute_main[] = R"(
void computemain();

void main() {
	computemain();
}
)";

struct StageInfo
{
	const char *name;
	const char *header;
	const char *uniforms;
	const char *functions;
	const char *main;
	const char *main_custom;
	const char *main_raw;
};

static const StageInfo stageInfo[] =
{
	{ "VERTEX", vertex_header, render_uniforms, vertex_functions, vertex_main, vertex_main, vertex_main_raw },
	{ "PIXEL", pixel_header, render_uniforms, pixel_functions, pixel_main, pixel_main_custom, pixel_main_raw },
	{ "COMPUTE", compute_header, compute_uniforms, compute_functions, compute_main, compute_main, compute_main },
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
	{ "#version 430 core", "#version 320 es" },
};

enum CommentType
{
	COMMENT_NONE,
	COMMENT_LINE,
	COMMENT_BLOCK,
};

static void parseComments(const std::string &src, std::vector<Range> &comments)
{
	CommentType commenttype = COMMENT_NONE;
	Range comment;

	const char *srcbytes = src.data();
	size_t len = src.length();

	for (size_t i = 0; i < len; i++)
	{
		char curchar = srcbytes[i];

		if (commenttype == COMMENT_NONE)
		{
			if (curchar == '/' && i + 1 < len)
			{
				char nextchar = srcbytes[i + 1];
				if (nextchar == '/')
				{
					commenttype = COMMENT_LINE;
					comment = Range(i, 1);
				}
				else if (nextchar == '*')
				{
					commenttype = COMMENT_BLOCK;
					comment = Range(i, 1);
				}
			}
		}
		else if (commenttype == COMMENT_LINE)
		{
			if (curchar == '\n')
			{
				commenttype = COMMENT_NONE;
				comment.last = i;
				comments.push_back(comment);
			}
		}
		else if (commenttype == COMMENT_BLOCK)
		{
			if (curchar == '/' && i > 0 && srcbytes[i - 1] == '*')
			{
				commenttype = COMMENT_NONE;
				comment.last = i;
				comments.push_back(comment);
			}
		}
	}

	if (commenttype == COMMENT_LINE)
	{
		comment.last = len - 1;
		comments.push_back(comment);
	}
}

static bool inComment(size_t i, const std::vector<Range> &comments)
{
	Range r(i, 1);

	for (const Range &comment : comments)
	{
		if (comment.contains(r))
			return true;
	}

	return false;
}

static bool textSearch(const std::string &src, const std::string &str, const std::vector<Range> &comments)
{
	size_t start = 0;
	size_t found = std::string::npos;

	while ((found = src.find(str, start)) != std::string::npos)
	{
		if (!inComment(found, comments))
			return true;
		start = found + str.size();
	}

	return false;
}

static bool regexSearch(const std::string &src, const std::string &rstr, const std::vector<Range> &comments)
{
	std::regex r(rstr);

	for (auto it = std::sregex_iterator(src.begin(), src.end(), r); it != std::sregex_iterator(); it++)
	{
		const std::smatch &m = *it;
		if (!inComment(m.position(), comments))
			return true;
	}

	return false;
}

static Shader::Language getTargetLanguage(const std::string &src)
{
	std::regex r("^\\s*#pragma language (\\w+)");
	std::smatch m;
	std::string langstr = std::regex_search(src, m, r) && m.size() > 1 ? m[1] : std::string("glsl1");
	Shader::Language lang = Shader::LANGUAGE_MAX_ENUM;
	Shader::getConstant(langstr.c_str(), lang);
	return lang;
}

static Shader::EntryPoint getVertexEntryPoint(const std::string &src, const std::vector<Range> &comments)
{
	if (regexSearch(src, "void\\s+vertexmain\\s*\\(", comments))
		return Shader::ENTRYPOINT_RAW;

	if (regexSearch(src, "vec4\\s+position\\s*\\(", comments))
		return Shader::ENTRYPOINT_HIGHLEVEL;

	return Shader::ENTRYPOINT_NONE;
}

static Shader::EntryPoint getPixelEntryPoint(const std::string &src, const std::vector<Range> &comments, bool &mrt)
{
	mrt = false;

	if (regexSearch(src, "void\\s+pixelmain\\s*\\(", comments))
		return Shader::ENTRYPOINT_RAW;

	if (regexSearch(src, "vec4\\s+effect\\s*\\(", comments))
		return Shader::ENTRYPOINT_HIGHLEVEL;

	if (regexSearch(src, "void\\s+effect\\s*\\(", comments))
	{
		if (textSearch(src, "love_RenderTargets", comments) || textSearch(src, "love_Canvases", comments))
			mrt = true;
		return Shader::ENTRYPOINT_CUSTOM;
	}

	return Shader::ENTRYPOINT_NONE;
}

static Shader::EntryPoint getComputeEntryPoint(const std::string &src, const std::vector<Range> &comments)
{
	if (regexSearch(src, "void\\s+computemain\\s*\\(", comments))
		return Shader::ENTRYPOINT_RAW;

	return Shader::ENTRYPOINT_NONE;
}

} // glsl

static_assert(sizeof(Shader::BuiltinUniformData) == sizeof(float) * 4 * 13, "Update the array in wrap_GraphicsShader.lua if this changes.");

love::Type Shader::type("Shader", &Object::type);

Shader *Shader::current = nullptr;
Shader *Shader::standardShaders[Shader::STANDARD_MAX_ENUM] = {nullptr};

Shader::SourceInfo Shader::getSourceInfo(const std::string &src)
{
	std::vector<Range> comments;
	glsl::parseComments(src, comments);

	SourceInfo info = {};
	info.language = glsl::getTargetLanguage(src);
	info.stages[SHADERSTAGE_VERTEX] = glsl::getVertexEntryPoint(src, comments);
	info.stages[SHADERSTAGE_PIXEL] = glsl::getPixelEntryPoint(src, comments, info.usesMRT);
	info.stages[SHADERSTAGE_COMPUTE] = glsl::getComputeEntryPoint(src, comments);
	if (info.stages[SHADERSTAGE_COMPUTE])
		info.language = LANGUAGE_GLSL4;
	return info;
}

std::string Shader::createShaderStageCode(Graphics *gfx, ShaderStageType stage, const std::string &code, const CompileOptions &options, const Shader::SourceInfo &info, bool gles, bool checksystemfeatures)
{
	if (info.language == Shader::LANGUAGE_MAX_ENUM)
		throw love::Exception("Invalid shader language");

	if (info.stages[stage] == ENTRYPOINT_NONE)
		throw love::Exception("Cannot find entry point for shader stage.");

	if (info.stages[stage] == ENTRYPOINT_RAW && info.language == LANGUAGE_GLSL1)
		throw love::Exception("Shaders using a raw entry point (vertexmain or pixelmain) must use GLSL 3 or greater.");

	if (stage == SHADERSTAGE_COMPUTE && info.language != LANGUAGE_GLSL4)
		throw love::Exception("Compute shaders must use GLSL 4.");

	bool glsl1on3 = info.language == LANGUAGE_GLSL1;

	if (checksystemfeatures)
	{
		const auto &features = gfx->getCapabilities().features;

		if (stage == SHADERSTAGE_COMPUTE && !features[Graphics::FEATURE_GLSL4])
			throw love::Exception("Compute shaders require GLSL 4 which is not supported on this system.");

		if (info.language == LANGUAGE_GLSL3 && !features[Graphics::FEATURE_GLSL3])
			throw love::Exception("GLSL 3 shaders are not supported on this system.");

		if (info.language == LANGUAGE_GLSL4 && !features[Graphics::FEATURE_GLSL4])
			throw love::Exception("GLSL 4 shaders are not supported on this system.");

		glsl1on3 = info.language == LANGUAGE_GLSL1 && features[Graphics::FEATURE_GLSL3];
	}

	Language lang = info.language;
	if (glsl1on3)
		lang = LANGUAGE_GLSL3;

	glsl::StageInfo stageinfo = glsl::stageInfo[stage];

	std::stringstream ss;

	ss << (gles ? glsl::versions[lang].glsles : glsl::versions[lang].glsl) << "\n";
	if (glsl1on3)
		ss << "#define LOVE_GLSL1_ON_GLSL3 1\n";

	if (isGammaCorrect())
		ss << "#define LOVE_GAMMA_CORRECT 1\n";
	if (info.usesMRT)
		ss << "#define LOVE_MULTI_RENDER_TARGETS 1\n";

	// Note: backends are expected to handle this situation if highp is ever
	// conditional in that backend.
	if (!gfx->getCapabilities().features[Graphics::FEATURE_PIXEL_SHADER_HIGHP])
		ss << "#define LOVE_SPLIT_UNIFORMS_PER_DRAW 1\n";

	for (const auto &def : options.defines)
		ss << "#define " + def.first + " " + def.second + "\n";

	ss << "#define " << stageinfo.name << " " << stageinfo.name << "\n";
	ss << glsl::global_syntax;
	ss << stageinfo.header;
	ss << stageinfo.uniforms;
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

Shader::Shader(StrongRef<ShaderStage> _stages[], const CompileOptions &options)
	: stages()
	, debugName(options.debugName)
{
	std::string err;
	if (!validateInternal(_stages, err, validationReflection))
		throw love::Exception("%s", err.c_str());

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
		stages[i] = _stages[i];
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

const Shader::UniformInfo *Shader::getMainTextureInfo() const
{
	return getUniformInfo(BUILTIN_TEXTURE_MAIN);
}

DataBaseType Shader::getDataBaseType(PixelFormat format)
{
	switch (getPixelFormatInfo(format).dataType)
	{
		case PIXELFORMATTYPE_UNORM:
			return DATA_BASETYPE_UNORM;
		case PIXELFORMATTYPE_SNORM:
			return DATA_BASETYPE_SNORM;
		case PIXELFORMATTYPE_UFLOAT:
		case PIXELFORMATTYPE_SFLOAT:
			return DATA_BASETYPE_FLOAT;
		case PIXELFORMATTYPE_SINT:
			return DATA_BASETYPE_INT;
		case PIXELFORMATTYPE_UINT:
			return DATA_BASETYPE_UINT;
		default:
			return DATA_BASETYPE_FLOAT;
	}
}

bool Shader::isResourceBaseTypeCompatible(DataBaseType a, DataBaseType b)
{
	if (a == DATA_BASETYPE_FLOAT || a == DATA_BASETYPE_UNORM || a == DATA_BASETYPE_SNORM)
		return b == DATA_BASETYPE_FLOAT || b == DATA_BASETYPE_UNORM || b == DATA_BASETYPE_SNORM;

	if (a == DATA_BASETYPE_INT && b == DATA_BASETYPE_INT)
		return true;

	if (a == DATA_BASETYPE_UINT && b == DATA_BASETYPE_UINT)
		return true;

	return false;
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

	if (!isResourceBaseTypeCompatible(info->dataBaseType, getDataBaseType(maintex->getPixelFormat())))
		throw love::Exception("Texture's data format base type must match the uniform variable declared in the shader (float, int, or uint).");

	if (info->isDepthSampler != maintex->getSamplerState().depthSampleMode.hasValue)
	{
		if (info->isDepthSampler)
			throw love::Exception("Depth comparison samplers in shaders can only be used with depth textures which have depth comparison set.");
		else
			throw love::Exception("Depth textures which have depth comparison set can only be used with depth/shadow samplers in shaders.");
	}
}

void Shader::getLocalThreadgroupSize(int *x, int *y, int *z)
{
	*x = validationReflection.localThreadgroupSize[0];
	*y = validationReflection.localThreadgroupSize[1];
	*z = validationReflection.localThreadgroupSize[2];
}

bool Shader::validate(StrongRef<ShaderStage> stages[], std::string& err)
{
	ValidationReflection reflection;
	return validateInternal(stages, err, reflection);
}

static PixelFormat getPixelFormat(glslang::TLayoutFormat format)
{
	using namespace glslang;

	switch (format)
	{
		case ElfNone: return PIXELFORMAT_UNKNOWN;
		case ElfRgba32f: return PIXELFORMAT_RGBA32_FLOAT;
		case ElfRgba16f: return PIXELFORMAT_RGBA16_FLOAT;
		case ElfR32f: return PIXELFORMAT_R32_FLOAT;
		case ElfRgba8: return PIXELFORMAT_RGBA8_UNORM;
		case ElfRgba8Snorm: return PIXELFORMAT_UNKNOWN; // no snorm yet
		case ElfRg32f: return PIXELFORMAT_RG32_FLOAT;
		case ElfRg16f: return PIXELFORMAT_RG16_FLOAT;
		case ElfR11fG11fB10f: return PIXELFORMAT_RG11B10_FLOAT;
		case ElfR16f: return PIXELFORMAT_R16_FLOAT;
		case ElfRgba16: return PIXELFORMAT_RGBA16_UNORM;
		case ElfRgb10A2: return PIXELFORMAT_RGB10A2_UNORM;
		case ElfRg16: return PIXELFORMAT_RG16_UNORM;
		case ElfRg8: return PIXELFORMAT_RG8_UNORM;
		case ElfR8: return PIXELFORMAT_R8_UNORM;
		case ElfRgba16Snorm: return PIXELFORMAT_UNKNOWN;
		case ElfRg16Snorm: return PIXELFORMAT_UNKNOWN;
		case ElfRg8Snorm: return PIXELFORMAT_UNKNOWN;
		case ElfR16Snorm: return PIXELFORMAT_UNKNOWN;
		case ElfR8Snorm: return PIXELFORMAT_UNKNOWN;
		case ElfRgba32i: return PIXELFORMAT_RGBA32_INT;
		case ElfRgba16i: return PIXELFORMAT_RGBA16_INT;
		case ElfRgba8i: return PIXELFORMAT_RGBA8_INT;
		case ElfR32i: return PIXELFORMAT_R32_INT;
		case ElfRg32i: return PIXELFORMAT_RG32_INT;
		case ElfRg16i: return PIXELFORMAT_RG16_INT;
		case ElfRg8i: return PIXELFORMAT_RG8_INT;
		case ElfR16i: return PIXELFORMAT_R16_INT;
		case ElfR8i: return PIXELFORMAT_R8_INT;
		case ElfRgba32ui: return PIXELFORMAT_RGBA32_UINT;
		case ElfRgba16ui: return PIXELFORMAT_RGBA16_UINT;
		case ElfRgba8ui: return PIXELFORMAT_RGBA8_UINT;
		case ElfR32ui: return PIXELFORMAT_R32_UINT;
		case ElfRg32ui: return PIXELFORMAT_RG32_UINT;
		case ElfRg16ui: return PIXELFORMAT_RG16_UINT;
		case ElfRgb10a2ui: return PIXELFORMAT_UNKNOWN;
		case ElfRg8ui: return PIXELFORMAT_RG8_UINT;
		case ElfR16ui: return PIXELFORMAT_R16_UINT;
		case ElfR8ui: return PIXELFORMAT_R8_UINT;
		default: return PIXELFORMAT_UNKNOWN;
	}
}

template <typename T>
static T convertData(const glslang::TConstUnion &data)
{
	switch (data.getType())
	{
		case glslang::EbtInt: return (T) data.getIConst();
		case glslang::EbtUint: return (T) data.getUConst();
		case glslang::EbtDouble: return (T) data.getDConst();
		case glslang::EbtInt8: return (T) data.getI8Const();
		case glslang::EbtInt16: return (T) data.getI16Const();
		case glslang::EbtInt64: return (T) data.getI64Const();
		case glslang::EbtUint8: return (T) data.getU8Const();
		case glslang::EbtUint16: return (T) data.getU16Const();
		case glslang::EbtUint64: return (T) data.getU64Const();
		default: return 0;
	}
}

bool Shader::validateInternal(StrongRef<ShaderStage> stages[], std::string &err, ValidationReflection &reflection)
{
	glslang::TProgram program;

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		if (stages[i] != nullptr)
			program.addShader(stages[i]->getGLSLangValidationShader());
	}

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

	if (stages[SHADERSTAGE_COMPUTE] != nullptr)
	{
		for (int i = 0; i < 3; i++)
		{
			reflection.localThreadgroupSize[i] = program.getLocalSize(i);

			if (reflection.localThreadgroupSize[i] <= 0)
			{
				err = "Shader validation error:\nNegative local threadgroup size.";
				return false;
			}
		}
	}

	for (int i = 0; i < program.getNumUniformVariables(); i++)
	{
		const glslang::TObjectReflection &info = program.getUniform(i);
		const glslang::TType *type = info.getType();
		if (type == nullptr)
			continue;

		const glslang::TQualifier &qualifiers = type->getQualifier();

		if (type->isImage())
		{
			if ((info.stages & EShLangComputeMask) == 0)
			{
				err = "Shader validation error:\nStorage Texture uniform variables (image2D, etc) are only allowed in compute shaders.";
				return false;
			}

			if (!qualifiers.hasFormat())
			{
				err = "Shader validation error:\nStorage Texture '" + info.name + "' must have an explicit format set in its layout declaration.";
				return false;
			}

			StorageTextureReflection texreflection = {};

			texreflection.format = getPixelFormat(qualifiers.getFormat());

			if (qualifiers.isReadOnly())
				texreflection.access = ACCESS_READ;
			else if (qualifiers.isWriteOnly())
				texreflection.access = ACCESS_WRITE;
			else
				texreflection.access = (Access)(ACCESS_READ | ACCESS_WRITE);

			reflection.storageTextures[info.name] = texreflection;
		}
		else if (!type->isOpaque())
		{
			LocalUniform u = {};
			auto &values = u.initializerValues;
			const glslang::TConstUnionArray *constarray = info.getConstArray();

			// Store initializer values for local uniforms. Some love graphics
			// backends strip these out of the shader so we need to be able to
			// access them (to re-send them) by getting them here.
			switch (type->getBasicType())
			{
			case glslang::EbtFloat:
				u.dataType = DATA_BASETYPE_FLOAT;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].f = convertData<float>((*constarray)[i]);
				}
				break;
			case glslang::EbtUint:
				u.dataType = DATA_BASETYPE_UINT;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].u = convertData<uint32>((*constarray)[i]);
				}
				break;
			case glslang::EbtBool:
				u.dataType = DATA_BASETYPE_BOOL;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].u = convertData<uint32>((*constarray)[i]);
				}
				break;
			case glslang::EbtInt:
			default:
				u.dataType = DATA_BASETYPE_INT;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].i = convertData<int32>((*constarray)[i]);
				}
				break;
			}

			reflection.localUniforms[info.name] = u;
		}
	}

	for (int i = 0; i < program.getNumBufferBlocks(); i++)
	{
		const glslang::TObjectReflection &info = program.getBufferBlock(i);
		const glslang::TType *type = info.getType();
		if (type != nullptr)
		{
			const glslang::TQualifier &qualifiers = type->getQualifier();

			if ((!qualifiers.isReadOnly() || qualifiers.isWriteOnly()) && (info.stages & EShLangComputeMask) == 0)
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
				err = "Shader validation error:\nStorage Buffer block '" + info.name + "' must contain a single unsized array of base types or structs.";
				return false;
			}

			const glslang::TType* elementtype = (*structure)[0].type;
			if (elementtype == nullptr || !elementtype->isUnsizedArray())
			{
				err = "Shader validation error:\nStorage Buffer block '" + info.name + "' must contain a single unsized array of base types or structs.";
				return false;
			}

			BufferReflection bufferReflection = {};
			bufferReflection.stride = (size_t) info.size;
			bufferReflection.memberCount = (size_t) info.numMembers;

			if (qualifiers.isReadOnly())
				bufferReflection.access = ACCESS_READ;
			else if (qualifiers.isWriteOnly())
				bufferReflection.access = ACCESS_WRITE;
			else
				bufferReflection.access = (Access)(ACCESS_READ | ACCESS_WRITE);

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

bool Shader::validateTexture(const UniformInfo *info, Texture *tex, bool internalUpdate)
{
	const SamplerState &sampler = tex->getSamplerState();
	bool isstoragetex = info->baseType == UNIFORM_STORAGETEXTURE;

	if (!tex->isReadable())
	{
		if (internalUpdate)
			return false;
		else
			throw love::Exception("Textures with non-readable formats cannot be sampled from in a shader.");
	}
	else if (info->isDepthSampler != sampler.depthSampleMode.hasValue)
	{
		if (internalUpdate)
			return false;
		else if (info->isDepthSampler)
			throw love::Exception("Depth comparison samplers in shaders can only be used with depth textures which have depth comparison set.");
		else
			throw love::Exception("Depth textures which have depth comparison set can only be used with depth/shadow samplers in shaders.");
	}
	else if (tex->getTextureType() != info->textureType)
	{
		if (internalUpdate)
			return false;
		else
		{
			const char *textypestr = "unknown";
			const char *shadertextypestr = "unknown";
			Texture::getConstant(tex->getTextureType(), textypestr);
			Texture::getConstant(info->textureType, shadertextypestr);
			throw love::Exception("Texture's type (%s) must match the type of %s (%s).", textypestr, info->name.c_str(), shadertextypestr);
		}
	}
	else if (!isResourceBaseTypeCompatible(info->dataBaseType, getDataBaseType(tex->getPixelFormat())))
	{
		if (internalUpdate)
			return false;
		else
			throw love::Exception("Texture's data format base type must match the uniform variable declared in the shader (float, int, or uint).");
	}
	else if (isstoragetex && !tex->isComputeWritable())
	{
		if (internalUpdate)
			return false;
		else
			throw love::Exception("Texture must be created with the computewrite flag set to true in order to be used with a storage texture (image2D etc) shader uniform variable.");
	}
	else if (isstoragetex && info->storageTextureFormat != getLinearPixelFormat(tex->getPixelFormat()))
	{
		if (internalUpdate)
			return false;
		else
		{
			const char *texpfstr = "unknown";
			const char *shaderpfstr = "unknown";
			love::getConstant(getLinearPixelFormat(tex->getPixelFormat()), texpfstr);
			love::getConstant(info->storageTextureFormat, shaderpfstr);
			throw love::Exception("Texture's pixel format (%s) must match the shader uniform variable %s's pixel format (%s)", texpfstr, info->name.c_str(), shaderpfstr);
		}
	}

	return true;
}

bool Shader::validateBuffer(const UniformInfo *info, Buffer *buffer, bool internalUpdate)
{
	uint32 requiredtypeflags = 0;

	bool texelbinding = info->baseType == UNIFORM_TEXELBUFFER;
	bool storagebinding = info->baseType == UNIFORM_STORAGEBUFFER;

	if (texelbinding)
		requiredtypeflags = BUFFERUSAGEFLAG_TEXEL;
	else if (storagebinding)
		requiredtypeflags = BUFFERUSAGEFLAG_SHADER_STORAGE;

	if ((buffer->getUsageFlags() & requiredtypeflags) == 0)
	{
		if (internalUpdate)
			return false;
		else if (texelbinding)
			throw love::Exception("Shader uniform '%s' is a texel buffer, but the given Buffer was not created with texel buffer capabilities.", info->name.c_str());
		else if (storagebinding)
			throw love::Exception("Shader uniform '%s' is a shader storage buffer block, but the given Buffer was not created with shader storage buffer capabilities.", info->name.c_str());
		else
			throw love::Exception("Shader uniform '%s' does not match the types supported by the given Buffer.", info->name.c_str());
	}

	if (texelbinding)
	{
		DataBaseType basetype = buffer->getDataMember(0).info.baseType;
		if (!isResourceBaseTypeCompatible(basetype, info->dataBaseType))
		{
			if (internalUpdate)
				return false;
			else
				throw love::Exception("Texel buffer's data format base type must match the variable declared in the shader.");
		}
	}
	else if (storagebinding)
	{
		if (info->bufferStride != buffer->getArrayStride())
		{
			if (internalUpdate)
				return false;
			else
				throw love::Exception("Shader storage block '%s' has an array stride of %d bytes, but the given Buffer has an array stride of %d bytes.",
					info->name.c_str(), info->bufferStride, buffer->getArrayStride());
		}
		else if (info->bufferMemberCount != buffer->getDataMembers().size())
		{
			if (internalUpdate)
				return false;
			else
				throw love::Exception("Shader storage block '%s' has a struct with %d fields, but the given Buffer has a format with %d members.",
					info->name.c_str(), info->bufferMemberCount, buffer->getDataMembers().size());
		}
	}

	return true;
}

bool Shader::fillUniformReflectionData(UniformInfo &u)
{
	const auto &r = validationReflection;

	if (u.baseType == UNIFORM_STORAGETEXTURE)
	{
		const auto reflectionit = r.storageTextures.find(u.name);
		if (reflectionit != r.storageTextures.end())
		{
			u.storageTextureFormat = reflectionit->second.format;
			u.access = reflectionit->second.access;
			return true;
		}

		// No reflection info - maybe glslang was better at detecting dead code
		// than the driver's compiler?
		return false;
	}
	else if (u.baseType == UNIFORM_STORAGEBUFFER)
	{
		const auto reflectionit = r.storageBuffers.find(u.name);
		if (reflectionit != r.storageBuffers.end())
		{
			u.bufferStride = reflectionit->second.stride;
			u.bufferMemberCount = reflectionit->second.memberCount;
			u.access = reflectionit->second.access;
			return true;
		}

		return false;
	}

	return true;
}

std::string Shader::getShaderStageDebugName(ShaderStageType stage) const
{
	std::string name = debugName;

	if (!name.empty())
	{
		const char *stagename = "unknown";
		ShaderStage::getConstant(stage, stagename);
		name += " (" + std::string(stagename) + ")";
	}

	return name;
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
	{ "MainTex",               Shader::BUILTIN_TEXTURE_MAIN        },
	{ "love_VideoYChannel",    Shader::BUILTIN_TEXTURE_VIDEO_Y     },
	{ "love_VideoCbChannel",   Shader::BUILTIN_TEXTURE_VIDEO_CB    },
	{ "love_VideoCrChannel",   Shader::BUILTIN_TEXTURE_VIDEO_CR    },
	{ "love_UniformsPerDraw",  Shader::BUILTIN_UNIFORMS_PER_DRAW   },
	{ "love_UniformsPerDraw2", Shader::BUILTIN_UNIFORMS_PER_DRAW_2 },
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
