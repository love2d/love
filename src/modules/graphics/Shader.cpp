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

// LOVE
#include "Shader.h"
#include "Graphics.h"
#include "math/MathModule.h"
#include "common/Range.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/glslang/Public/ResourceLimits.h"

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
#define LOVE_HIGHP_OR_MEDIUMP highp
#define number float
#define Image sampler2D
#define ArrayImage sampler2DArray
#define CubeImage samplerCube
#define VolumeImage sampler3D
#define DepthImage sampler2DShadow
#define DepthArrayImage sampler2DArrayShadow
#define DepthCubeImage samplerCubeShadow
#define extern uniform

#if __VERSION__ >= 430 || (defined(GL_ES) && __VERSION__ >= 310)
	layout (std430) buffer;
#endif
)";

static const char render_uniforms[] = R"(
uniform highp vec4 love_UniformsPerDraw[12];

// Older GLSL doesn't support preprocessor line continuations...
#define TransformMatrix mat4(love_UniformsPerDraw[0], love_UniformsPerDraw[1], love_UniformsPerDraw[2], love_UniformsPerDraw[3])
#define ProjectionMatrix mat4(love_UniformsPerDraw[4], love_UniformsPerDraw[5], love_UniformsPerDraw[6], love_UniformsPerDraw[7])
#define TransformProjectionMatrix (ProjectionMatrix * TransformMatrix)

#define CurrentDPIScale (love_UniformsPerDraw[8].x)
#define ConstantPointSize (love_UniformsPerDraw[8].y)
#define love_ClipSpaceParams (love_UniformsPerDraw[9])
#define ConstantColor (love_UniformsPerDraw[10])
#define love_ScreenSize (love_UniformsPerDraw[11])

// Alternate names
#define ViewSpaceFromLocal TransformMatrix
#define ClipSpaceFromView ProjectionMatrix
#define ClipSpaceFromLocal TransformProjectionMatrix
)";

static const char global_functions[] = R"(
#ifdef GL_ES
	precision mediump sampler2D;
	precision mediump sampler2DArray;
	precision mediump sampler3D;
	precision mediump samplerCube;
	precision mediump sampler2DShadow;
	precision mediump samplerCubeShadow;
	precision mediump sampler2DArrayShadow;

	precision highp isampler2D;
	precision highp isampler2DArray;
	precision highp isampler3D;
	precision highp isamplerCube;

	precision highp usampler2D;
	precision highp usampler2DArray;
	precision highp usampler3D;
	precision highp usamplerCube;
#endif

// Avoid #define so legacy code that uses 'texture' as a variable can work...
// Unfortunately it means these can't have variable precision.
vec4 Texel(sampler2D s, highp vec2 c) { return texture(s, c); }
ivec4 Texel(isampler2D s, highp vec2 c) { return texture(s, c); }
uvec4 Texel(usampler2D s, highp vec2 c) { return texture(s, c); }

vec4 Texel(sampler3D s, highp vec3 c) { return texture(s, c); }
ivec4 Texel(isampler3D s, highp vec3 c) { return texture(s, c); }
uvec4 Texel(usampler3D s, highp vec3 c) { return texture(s, c); }

vec4 Texel(samplerCube s, highp vec3 c) { return texture(s, c); }
ivec4 Texel(isamplerCube s, highp vec3 c) { return texture(s, c); }
uvec4 Texel(usamplerCube s, highp vec3 c) { return texture(s, c); }

vec4 Texel(sampler2DArray s, highp vec3 c) { return texture(s, c); }
ivec4 Texel(isampler2DArray s, highp vec3 c) { return texture(s, c); }
uvec4 Texel(usampler2DArray s, highp vec3 c) { return texture(s, c); }

float Texel(sampler2DShadow s, highp vec3 c) { return texture(s, c); }
float Texel(samplerCubeShadow s, highp vec4 c) { return texture(s, c); }
float Texel(sampler2DArrayShadow s, highp vec4 c) { return texture(s, c); }

#ifdef PIXEL
	vec4 Texel(sampler2D s, highp vec2 c, float b) { return texture(s, c, b); }
	ivec4 Texel(isampler2D s, highp vec2 c, float b) { return texture(s, c, b); }
	uvec4 Texel(usampler2D s, highp vec2 c, float b) { return texture(s, c, b); }

	vec4 Texel(sampler3D s, highp vec3 c, float b) { return texture(s, c, b); }
	ivec4 Texel(isampler3D s, highp vec3 c, float b) { return texture(s, c, b); }
	uvec4 Texel(usampler3D s, highp vec3 c, float b) { return texture(s, c, b); }

	vec4 Texel(samplerCube s, highp vec3 c, float b) { return texture(s, c, b); }
	ivec4 Texel(isamplerCube s, highp vec3 c, float b) { return texture(s, c, b); }
	uvec4 Texel(usamplerCube s, highp vec3 c, float b) { return texture(s, c, b); }

	vec4 Texel(sampler2DArray s, highp vec3 c, float b) { return texture(s, c, b); }
	ivec4 Texel(isampler2DArray s, highp vec3 c, float b) { return texture(s, c, b); }
	uvec4 Texel(usampler2DArray s, highp vec3 c, float b) { return texture(s, c, b); }

	float Texel(sampler2DShadow s, highp vec3 c, float b) { return texture(s, c, b); }
#ifndef LOVE_NO_TEXTURECUBESHADOWBIAS_HACK
	float Texel(samplerCubeShadow s, highp vec4 c, float b) { return texture(s, c, b); }
#endif
#endif

uniform mediump float deprecatedTextureCall;

vec4 texture2DDeprecated(sampler2D s, vec2 c) { return texture(s, c) + deprecatedTextureCall; }
vec4 textureCubeDeprecated(samplerCube s, vec3 c) { return texture(s, c) + deprecatedTextureCall; }

#ifdef PIXEL
vec4 texture2DDeprecated(sampler2D s, vec2 c, float b) { return texture(s, c, b) + deprecatedTextureCall; }
vec4 textureCubeDeprecated(samplerCube s, vec3 c, float b) { return texture(s, c, b) + deprecatedTextureCall; }
#endif

#define texture2D texture2DDeprecated
#define textureCube textureCubeDeprecated

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

#define attribute in
#define varying out
#define love_VertexID gl_VertexID
#define love_InstanceID gl_InstanceID
)";

static const char vertex_functions[] = R"(
vec4 love_clipSpaceTransform(vec4 clipPosition) {
	clipPosition.y *= love_ClipSpaceParams.x;
	clipPosition.z = (love_ClipSpaceParams.y * clipPosition.z + love_ClipSpaceParams.z * clipPosition.w) * love_ClipSpaceParams.w;
	return clipPosition;
}
)";

static const char vertex_main[] = R"(
layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexTexCoord;
layout (location = 2) in vec4 VertexColor;

out vec4 VaryingTexCoord;
out vec4 VaryingColor;

vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition);

void main() {
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;
	love_Position = position(ClipSpaceFromLocal, VertexPosition);
	love_Position = love_clipSpaceTransform(love_Position);
}
)";

static const char vertex_main_raw[] = R"(
void vertexmain();

void main() {
	vertexmain();
	love_Position = love_clipSpaceTransform(love_Position);
}
)";

static const char pixel_header[] = R"(
#ifdef GL_ES
	precision mediump float;
#endif

#define love_MaxRenderTargets gl_MaxDrawBuffers

#define varying in

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
layout (location = 0) out vec4 love_PixelColor;

uniform sampler2D MainTex;
in highp vec4 VaryingTexCoord;
in mediump vec4 VaryingColor;

vec4 effect(vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord);

void main() {
	love_PixelColor = effect(VaryingColor, MainTex, VaryingTexCoord.st, love_PixelCoord);
}
)";

static const char pixel_main_custom[] = R"(
// Some drivers seem to make the pixel shader do more work when multiple
// pixel shader outputs are defined, even when only one is actually used.
// TODO: We should use reflection or something instead of this, to determine
// how many outputs are actually used in the shader code.
#ifdef LOVE_MULTI_RENDER_TARGETS
	layout (location = 0) out vec4 love_RenderTargets[love_MaxRenderTargets];
	#define love_PixelColor love_RenderTargets[0]
#else
	layout (location = 0) out vec4 love_PixelColor;
#endif

// Legacy
#define love_Canvases love_RenderTargets
#ifdef LOVE_MULTI_RENDER_TARGETS
#define LOVE_MULTI_CANVASES 1
#endif

in highp vec4 VaryingTexCoord;
in mediump vec4 VaryingColor;

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
	std::string langstr = std::regex_search(src, m, r) && m.size() > 1 ? m[1] : std::string("glsl3");
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

static_assert(sizeof(Shader::BuiltinUniformData) == sizeof(float) * 4 * 12, "Update the array in wrap_GraphicsShader.lua if this changes.");

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

	if (stage == SHADERSTAGE_COMPUTE && info.language != LANGUAGE_GLSL4)
		throw love::Exception("Compute shaders must use GLSL 4.");

	if (checksystemfeatures)
	{
		const auto &features = gfx->getCapabilities().features;

		if (stage == SHADERSTAGE_COMPUTE && !features[Graphics::FEATURE_GLSL4])
			throw love::Exception("Compute shaders require GLSL 4 which is not supported on this system.");

		if (info.language == LANGUAGE_GLSL4 && !features[Graphics::FEATURE_GLSL4])
			throw love::Exception("GLSL 4 shaders are not supported on this system.");
	}

	Language lang = info.language;

	glsl::StageInfo stageinfo = glsl::stageInfo[stage];

	std::stringstream ss;

	ss << (gles ? glsl::versions[lang].glsles : glsl::versions[lang].glsl) << "\n";

	if (isGammaCorrect())
		ss << "#define LOVE_GAMMA_CORRECT 1\n";
	if (info.usesMRT)
		ss << "#define LOVE_MULTI_RENDER_TARGETS 1\n";

	if (gfx->isUsingNoTextureCubeShadowBiasHack())
		ss << "#define LOVE_NO_TEXTURECUBESHADOWBIAS_HACK 1\n";

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
	ss << "#line 1\n";
	ss << code;

	return ss.str();
}

Shader::Shader(StrongRef<ShaderStage> _stages[], const CompileOptions &options)
	: stages()
	, debugName(options.debugName)
{
	std::string err;
	if (!validateInternal(_stages, err, reflection))
		throw love::Exception("%s", err.c_str());

	std::vector<std::string> unsetVertexInputLocations;

	for (const auto &kvp : reflection.vertexInputs)
	{
		if (kvp.second < 0)
			unsetVertexInputLocations.push_back(kvp.first);
	}

	if (!unsetVertexInputLocations.empty())
	{
		std::string str = unsetVertexInputLocations[0];
		for (size_t i = 1; i < unsetVertexInputLocations.size(); i++)
			str += ", " + unsetVertexInputLocations[i];
		unsetVertexInputLocationsString = str;
	}

	activeTextures.resize(reflection.textureCount);
	activeBuffers.resize(reflection.bufferCount);

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);

	// Default bindings for read-only resources.
	for (const auto &kvp : reflection.allUniforms)
	{
		const auto &u = *kvp.second;

		if (u.resourceIndex < 0)
			continue;

		if ((u.access & ACCESS_WRITE) != 0)
			continue;

		if (u.baseType == UNIFORM_SAMPLER || u.baseType == UNIFORM_STORAGETEXTURE)
		{
			auto tex = gfx->getDefaultTexture(u.textureType, u.dataBaseType, u.isDepthSampler);
			for (int i = 0; i < u.count; i++)
			{
				tex->retain();
				activeTextures[u.resourceIndex + i] = tex;
			}
		}
		else if (u.baseType == UNIFORM_TEXELBUFFER || u.baseType == UNIFORM_STORAGEBUFFER)
		{
			auto buffer = u.baseType == UNIFORM_TEXELBUFFER
				? gfx->getDefaultTexelBuffer(u.dataBaseType)
				: gfx->getDefaultStorageBuffer();

			for (int i = 0; i < u.count; i++)
			{
				buffer->retain();
				activeBuffers[u.resourceIndex + i] = buffer;
			}
		}
	}

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

	for (Texture *tex : activeTextures)
	{
		if (tex)
			tex->release();
	}

	for (Buffer *buffer : activeBuffers)
	{
		if (buffer)
			buffer->release();
	}
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

Vector4 Shader::computeClipSpaceParams(uint32 clipSpaceTransformFlags)
{
	// See the love_clipSpaceTransform vertex shader function.
	Vector4 params(1.0f, 1.0f, 0.0f, 1.0f);

	if (clipSpaceTransformFlags & CLIP_TRANSFORM_FLIP_Y)
		params.x = -1.0f;

	if (clipSpaceTransformFlags & CLIP_TRANSFORM_Z_NEG1_1_TO_0_1)
	{
		params.z = 1.0f;
		params.w = 0.5f;
	}
	else if (clipSpaceTransformFlags & CLIP_TRANSFORM_Z_0_1_TO_NEG1_1)
	{
		params.y = 2.0f;
		params.z = -1.0f;
	}

	return params;
}

const Shader::UniformInfo *Shader::getUniformInfo(const std::string &name) const
{
	const auto it = reflection.allUniforms.find(name);
	return it != reflection.allUniforms.end() ? it->second : nullptr;
}

bool Shader::hasUniform(const std::string &name) const
{
	const auto it = reflection.allUniforms.find(name);
	return it != reflection.allUniforms.end() && it->second->active;
}

void Shader::setVideoTextures(love::graphics::Texture *ytexture, love::graphics::Texture *cbtexture, love::graphics::Texture *crtexture)
{
	const BuiltinUniform builtins[3] = {
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
	};

	love::graphics::Texture *textures[3] = {ytexture, cbtexture, crtexture};

	for (int i = 0; i < 3; i++)
	{
		const UniformInfo *info = getUniformInfo(builtins[i]);
		if (info != nullptr)
			sendTextures(info, &textures[i], 1, true);
	}
}

void Shader::sendTextures(const UniformInfo *info, Texture **textures, int count)
{
	Shader::sendTextures(info, textures, count, false);
}

void Shader::sendBuffers(const UniformInfo *info, Buffer **buffers, int count)
{
	Shader::sendBuffers(info, buffers, count, false);
}

void Shader::sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalUpdate)
{
	UniformType basetype = info->baseType;

	if (basetype != UNIFORM_SAMPLER && basetype != UNIFORM_STORAGETEXTURE)
		return;

	if (!internalUpdate && current == this)
		flushBatchedDraws();

	count = std::min(count, info->count);

	for (int i = 0; i < count; i++)
	{
		love::graphics::Texture *tex = textures[i];
		bool isdefault = tex == nullptr;

		if (tex != nullptr)
		{
			if (!validateTexture(info, tex, internalUpdate))
				continue;
		}
		else
		{
			auto gfx = Module::getInstance<love::graphics::Graphics>(Module::M_GRAPHICS);
			tex = gfx->getDefaultTexture(info->textureType, info->dataBaseType, info->isDepthSampler);
		}

		tex->retain();

		int resourceindex = info->resourceIndex + i;

		if (activeTextures[resourceindex] != nullptr)
			activeTextures[resourceindex]->release();

		activeTextures[resourceindex] = tex;

		applyTexture(info, i, tex, basetype, isdefault);
	}
}

void Shader::sendBuffers(const UniformInfo *info, Buffer **buffers, int count, bool internalUpdate)
{
	UniformType basetype = info->baseType;

	if (basetype != UNIFORM_TEXELBUFFER && basetype != UNIFORM_STORAGEBUFFER)
		return;

	if (!internalUpdate && current == this)
		flushBatchedDraws();

	count = std::min(count, info->count);

	for (int i = 0; i < count; i++)
	{
		love::graphics::Buffer *buffer = buffers[i];
		bool isdefault = buffer == nullptr;

		if (buffer != nullptr)
		{
			if (!validateBuffer(info, buffer, internalUpdate))
				continue;
		}
		else
		{
			auto gfx = Module::getInstance<love::graphics::Graphics>(Module::M_GRAPHICS);
			if (basetype == UNIFORM_TEXELBUFFER)
				buffer = gfx->getDefaultTexelBuffer(info->dataBaseType);
			else
				buffer = gfx->getDefaultStorageBuffer();
		}

		buffer->retain();

		int resourceindex = info->resourceIndex + i;

		if (activeBuffers[resourceindex] != nullptr)
			activeBuffers[resourceindex]->release();

		activeBuffers[resourceindex] = buffer;

		applyBuffer(info, i, buffer, basetype, isdefault);
	}
}

void Shader::flushBatchedDraws() const
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();
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
	if ((primtype == PRIMITIVE_POINTS) != reflection.usesPointSize)
	{
		if (reflection.usesPointSize)
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
		throw love::Exception("Main texture's data format base type must match the MainTex declaration in the shader (float, int, or uint).");

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
	*x = reflection.localThreadgroupSize[0];
	*y = reflection.localThreadgroupSize[1];
	*z = reflection.localThreadgroupSize[2];
}

const std::vector<Buffer::DataDeclaration> *Shader::getBufferFormat(const std::string &name) const
{
	auto it = reflection.bufferFormats.find(name);
	if (it != reflection.bufferFormats.end())
		return &it->second;
	return nullptr;
}

bool Shader::isUsingDeprecatedTextureFunctions() const
{
	auto it = reflection.localUniforms.find("deprecatedTextureCall");
	return it != reflection.localUniforms.end() && it->second.stageMask != 0;
}

bool Shader::isUsingDeprecatedTextureUniform() const
{
	auto it = reflection.allUniforms.find("texture");
	return it != reflection.allUniforms.end() && it->second->stageMask != 0;
}

bool Shader::validate(StrongRef<ShaderStage> stages[], std::string& err)
{
	Reflection reflection;
	return validateInternal(stages, err, reflection);
}

static DataBaseType getBaseType(glslang::TBasicType basictype)
{
	switch (basictype)
	{
		case glslang::EbtInt: return DATA_BASETYPE_INT;
		case glslang::EbtUint: return DATA_BASETYPE_UINT;
		case glslang::EbtFloat: return DATA_BASETYPE_FLOAT;
		case glslang::EbtBool: return DATA_BASETYPE_BOOL;
		default: return DATA_BASETYPE_FLOAT;
	}
}

static DataFormat getDataFormat(glslang::TBasicType basictype, int components, int rows, int columns, bool matrix)
{
	if (matrix)
	{
		if (basictype != glslang::EbtFloat)
			return DATAFORMAT_MAX_ENUM;

		if (rows == 2 && columns == 2)
			return DATAFORMAT_FLOAT_MAT2X2;
		else if (rows == 2 && columns == 3)
			return DATAFORMAT_FLOAT_MAT2X3;
		else if (rows == 2 && columns == 4)
			return DATAFORMAT_FLOAT_MAT2X4;
		else if (rows == 3 && columns == 2)
			return DATAFORMAT_FLOAT_MAT3X2;
		else if (rows == 3 && columns == 3)
			return DATAFORMAT_FLOAT_MAT3X3;
		else if (rows == 3 && columns == 4)
			return DATAFORMAT_FLOAT_MAT3X4;
		else if (rows == 4 && columns == 2)
			return DATAFORMAT_FLOAT_MAT4X2;
		else if (rows == 4 && columns == 3)
			return DATAFORMAT_FLOAT_MAT4X3;
		else if (rows == 4 && columns == 4)
			return DATAFORMAT_FLOAT_MAT4X4;
		else
			return DATAFORMAT_MAX_ENUM;
	}
	else if (basictype == glslang::EbtFloat)
	{
		if (components == 1)
			return DATAFORMAT_FLOAT;
		else if (components == 2)
			return DATAFORMAT_FLOAT_VEC2;
		else if (components == 3)
			return DATAFORMAT_FLOAT_VEC3;
		else if (components == 4)
			return DATAFORMAT_FLOAT_VEC4;
	}
	else if (basictype == glslang::EbtInt)
	{
		if (components == 1)
			return DATAFORMAT_INT32;
		else if (components == 2)
			return DATAFORMAT_INT32_VEC2;
		else if (components == 3)
			return DATAFORMAT_INT32_VEC3;
		else if (components == 4)
			return DATAFORMAT_INT32_VEC4;
	}
	else if (basictype == glslang::EbtUint || basictype == glslang::EbtBool)
	{
		if (components == 1)
			return DATAFORMAT_UINT32;
		else if (components == 2)
			return DATAFORMAT_UINT32_VEC2;
		else if (components == 3)
			return DATAFORMAT_UINT32_VEC3;
		else if (components == 4)
			return DATAFORMAT_UINT32_VEC4;
	}

	return DATAFORMAT_MAX_ENUM;
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

static TextureType getTextureType(const glslang::TSampler &sampler)
{
	if (sampler.is2D())
		return sampler.isArrayed() ? TEXTURE_2D_ARRAY : TEXTURE_2D;
	else if (sampler.dim == glslang::EsdCube)
		return sampler.isArrayed() ? TEXTURE_MAX_ENUM : TEXTURE_CUBE;
	else if (sampler.dim == glslang::Esd3D)
		return TEXTURE_VOLUME;
	else
		return TEXTURE_MAX_ENUM;
}

static uint32 getStageMask(EShLanguageMask mask)
{
	uint32 m = 0;
	if (mask & EShLangVertexMask)
		m |= SHADERSTAGEMASK_VERTEX;
	if (mask & EShLangFragmentMask)
		m |= SHADERSTAGEMASK_PIXEL;
	if (mask & EShLangComputeMask)
		m |= SHADERSTAGEMASK_COMPUTE;
	return m;
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
		case glslang::EbtBool: return (T)data.getBConst();
		default: return 0;
	}
}

static bool AddFieldsToFormat(std::vector<Buffer::DataDeclaration> &format, int level, const glslang::TType *type, int arraylength, const std::string &basename, std::string &err)
{
	if (type->isStruct())
	{
		auto fields = type->getStruct();

		for (int i = 0; i < std::max(arraylength, 1); i++)
		{
			std::string name = basename;
			if (level > 0)
			{
				name += type->getFieldName().c_str();
				if (arraylength > 0)
					name += "[" + std::to_string(i) + "]";
				name += ".";
			}
			for (size_t fieldi = 0; fieldi < fields->size(); fieldi++)
			{
				const glslang::TType *fieldtype = (*fields)[fieldi].type;
				int fieldlength = fieldtype->isSizedArray() ? fieldtype->getCumulativeArraySize() : 0;

				if (!AddFieldsToFormat(format, level + 1, fieldtype, fieldlength, name, err))
					return false;
			}
		}
	}
	else
	{
		DataFormat dataformat = getDataFormat(type->getBasicType(), type->getVectorSize(), type->getMatrixRows(), type->getMatrixCols(), type->isMatrix());
		if (dataformat == DATAFORMAT_MAX_ENUM)
		{
			err = "Shader validation error:\nUnhandled data format for type " + (int)type->getBasicType() + std::string(" with name ") + basename;
			return false;
		}

		std::string name = basename.empty() ? type->getFieldName().c_str() : basename + type->getFieldName().c_str();
		format.emplace_back(name.c_str(), dataformat, arraylength);
	}

	return true;
}

bool Shader::validateInternal(StrongRef<ShaderStage> stages[], std::string &err, Reflection &reflection)
{
	glslang::TProgram program;

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		if (stages[i] != nullptr)
			program.addShader(stages[i]->getGLSLangValidationShader());
	}

	if (!program.link((EShMessages)(EshMsgCrossStageIO | EshMsgOverlappingLocations)))
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

	for (int i = 0; i < program.getNumPipeInputs(); i++)
	{
		const glslang::TObjectReflection &info = program.getPipeInput(i);

		// Avoid builtins.
		if (info.name.rfind("gl_", 0) == 0)
			continue;

		int location = info.layoutLocation();
		if (location == glslang::TQualifier::layoutLocationEnd)
			location = -1;

		reflection.vertexInputs[info.name] = location;
	}

	reflection.textureCount = 0;
	reflection.bufferCount = 0;

	for (int i = 0; i < program.getNumUniformVariables(); i++)
	{
		const glslang::TObjectReflection &info = program.getUniform(i);
		const glslang::TType *type = info.getType();
		if (type == nullptr)
			continue;

		const glslang::TQualifier &qualifiers = type->getQualifier();

		UniformInfo u = {};

		u.name = canonicaliizeUniformName(info.name);
		u.location = -1;
		u.access = ACCESS_READ;
		u.stageMask = getStageMask(info.stages);
		u.components = 1;
		u.resourceIndex = -1;

		if (type->isSizedArray())
			u.count = type->getArraySizes()->getCumulativeSize();
		else
			u.count = 1;

		const auto &sampler = type->getSampler();

		if (type->isTexture() && type->getSampler().isCombined() && !sampler.isBuffer())
		{
			u.baseType = UNIFORM_SAMPLER;
			u.dataBaseType = getBaseType(sampler.getBasicType());
			u.isDepthSampler = sampler.isShadow();
			u.textureType = getTextureType(sampler);

			if (u.textureType == TEXTURE_MAX_ENUM)
				continue;

			u.resourceIndex = reflection.textureCount;
			reflection.textureCount += u.count;

			reflection.sampledTextures[u.name] = u;
		}
		else if (type->isImage())
		{
			if ((info.stages & (~EShLangComputeMask)) != 0)
			{
				err = "Shader validation error:\nStorage Texture uniform variables (image2D, etc) are only allowed in compute shaders.";
				return false;
			}

			if (!qualifiers.hasFormat())
			{
				err = "Shader validation error:\nStorage Texture '" + u.name + "' must have an explicit format set in its layout declaration.";
				return false;
			}

			u.baseType = UNIFORM_STORAGETEXTURE;
			u.storageTextureFormat = getPixelFormat(qualifiers.getFormat());
			u.dataBaseType = getDataBaseType(u.storageTextureFormat);
			u.textureType = getTextureType(sampler);

			if (u.textureType == TEXTURE_MAX_ENUM)
				continue;

			u.resourceIndex = reflection.textureCount;
			reflection.textureCount += u.count;

			if (qualifiers.isReadOnly())
				u.access = ACCESS_READ;
			else if (qualifiers.isWriteOnly())
				u.access = ACCESS_WRITE;
			else
				u.access = (Access)(ACCESS_READ | ACCESS_WRITE);

			reflection.storageTextures[u.name] = u;
		}
		else if (type->getBasicType() == glslang::EbtSampler && sampler.isBuffer())
		{
			u.baseType = UNIFORM_TEXELBUFFER;
			u.dataBaseType = getBaseType(sampler.getBasicType());

			u.resourceIndex = reflection.bufferCount;
			reflection.bufferCount += u.count;

			reflection.texelBuffers[u.name] = u;
		}
		else if (!type->isOpaque())
		{
			std::vector<LocalUniformValue> values;
			const glslang::TConstUnionArray *constarray = info.getConstArray();

			if (type->isMatrix())
			{
				u.matrix.rows = type->getMatrixRows();
				u.matrix.columns = type->getMatrixCols();
			}
			else
			{
				u.components = type->getVectorSize();
			}

			// Store initializer values for local uniforms. Some love graphics
			// backends strip these out of the shader so we need to be able to
			// access them (to re-send them) by getting them here.
			switch (type->getBasicType())
			{
			case glslang::EbtFloat:
				u.baseType = type->isMatrix() ? UNIFORM_MATRIX : UNIFORM_FLOAT;
				u.dataBaseType = DATA_BASETYPE_FLOAT;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].f = convertData<float>((*constarray)[i]);
				}
				break;
			case glslang::EbtUint:
				u.baseType = UNIFORM_UINT;
				u.dataBaseType = DATA_BASETYPE_UINT;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].u = convertData<uint32>((*constarray)[i]);
				}
				break;
			case glslang::EbtBool:
				u.baseType = UNIFORM_BOOL;
				u.dataBaseType = DATA_BASETYPE_BOOL;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].u = convertData<uint32>((*constarray)[i]);
				}
				break;
			case glslang::EbtInt:
			default:
				u.baseType = UNIFORM_INT;
				u.dataBaseType = DATA_BASETYPE_INT;
				if (constarray != nullptr)
				{
					values.resize(constarray->size());
					for (int i = 0; i < constarray->size(); i++)
						values[i].i = convertData<int32>((*constarray)[i]);
				}
				break;
			}

			reflection.localUniforms[u.name] = u;
			reflection.localUniformInitializerValues[u.name] = values;
		}
	}

	for (int i = 0; i < program.getNumBufferBlocks(); i++)
	{
		const glslang::TObjectReflection &info = program.getBufferBlock(i);
		const glslang::TType *type = info.getType();
		if (type != nullptr)
		{
			const glslang::TQualifier &qualifiers = type->getQualifier();

			if ((!qualifiers.isReadOnly() || qualifiers.isWriteOnly()) && ((info.stages & (~EShLangComputeMask)) != 0))
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

			UniformInfo u = {};
			u.name = canonicaliizeUniformName(info.name);
			u.location = -1;
			u.stageMask = getStageMask(info.stages);
			u.components = 1;
			u.baseType = UNIFORM_STORAGEBUFFER;

			if (type->isSizedArray())
				u.count = type->getArraySizes()->getCumulativeSize();
			else
				u.count = 1;

			u.bufferStride = (size_t) info.size;
			u.bufferMemberCount = (size_t) info.numMembers;

			u.resourceIndex = reflection.bufferCount;
			reflection.bufferCount += u.count;

			if (qualifiers.isReadOnly())
				u.access = ACCESS_READ;
			else if (qualifiers.isWriteOnly())
				u.access = ACCESS_WRITE;
			else
				u.access = (Access)(ACCESS_READ | ACCESS_WRITE);

			reflection.storageBuffers[u.name] = u;

			std::vector<Buffer::DataDeclaration> format;
			if (!AddFieldsToFormat(format, 0, elementtype, 0, "", err))
				return false;

			reflection.bufferFormats[u.name] = format;
		}
		else
		{
			err = "Shader validation error:\nCannot retrieve type information for Storage Buffer Block '" + info.name + "'.";
			return false;
		}
	}

	for (auto &kvp : reflection.texelBuffers)
		reflection.allUniforms[kvp.first] = &kvp.second;

	for (auto &kvp : reflection.storageBuffers)
		reflection.allUniforms[kvp.first] = &kvp.second;

	for (auto &kvp : reflection.sampledTextures)
		reflection.allUniforms[kvp.first] = &kvp.second;

	for (auto &kvp : reflection.storageTextures)
		reflection.allUniforms[kvp.first] = &kvp.second;

	for (auto &kvp : reflection.localUniforms)
		reflection.allUniforms[kvp.first] = &kvp.second;

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
	else if (isstoragetex && info->storageTextureFormat != tex->getPixelFormat())
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
			// Don't prevent this from working for internally bound default resources.
			if (!internalUpdate)
				throw love::Exception("Shader storage block '%s' has an array stride of %d bytes, but the given Buffer has an array stride of %d bytes.",
					info->name.c_str(), info->bufferStride, buffer->getArrayStride());
		}
		else if (info->bufferMemberCount != buffer->getDataMembers().size())
		{
			// Don't prevent this from working for internally bound default resources.
			if (!internalUpdate)
				throw love::Exception("Shader storage block '%s' has a struct with %d fields, but the given Buffer has a format with %d members.",
					info->name.c_str(), info->bufferMemberCount, buffer->getDataMembers().size());
		}
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

std::string Shader::canonicaliizeUniformName(const std::string &n)
{
	std::string name(n);

	// Some drivers/compilers append "[0]" to the end of array uniform names.
	if (name.length() > 3)
	{
		size_t findpos = name.rfind("[0]");
		if (findpos != std::string::npos && findpos == name.length() - 3)
			name.erase(name.length() - 3);
	}

	return name;
}

void Shader::handleUnknownUniformName(const char */*name*/)
{
	// TODO: do something here?
}

void Shader::copyToUniformBuffer(const UniformInfo *info, const void *src, void *dst, int count) const
{
	count = std::min(count, info->count);

	size_t elementsize = info->components * 4;
	if (info->baseType == UNIFORM_MATRIX)
		elementsize = info->matrix.columns * info->matrix.rows * 4;

	// Assuming std140 packing rules, the source data can only be direct-copied
	// to the uniform buffer in certain cases because it's tightly packed whereas
	// the buffer's data isn't.
	if (elementsize * info->count == info->dataSize || (count == 1 && info->baseType != UNIFORM_MATRIX))
	{
		memcpy(dst, src, elementsize * count);
	}
	else
	{
		int veccount = count;
		int comp = info->components;

		if (info->baseType == UNIFORM_MATRIX)
		{
			veccount *= info->matrix.rows;
			comp = info->matrix.columns;
		}

		const int *isrc = (const int *) src;
		int *idst = (int *) dst;

		for (int i = 0; i < veccount; i++)
		{
			for (int c = 0; c < comp; c++)
				idst[i * 4 + c] = isrc[i * comp + c];
		}
	}
}

bool Shader::initialize()
{
	bool success = glslang::InitializeProcess();
	if (!success)
		return false;

	TBuiltInResource *resources = GetResources();
	*resources = *GetDefaultResources();

	// This is 32 in the default resource struct, which is too high for Metal.
	// TODO: Set this based on what the system actually supports?
	resources->maxDrawBuffers = 8;

	return true;
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
	{ "glsl3", Shader::LANGUAGE_GLSL3 },
	{ "glsl4", Shader::LANGUAGE_GLSL4 },
};

static StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM> languages(languageEntries, sizeof(languageEntries));

static StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry builtinNameEntries[] =
{
	{ "MainTex",               Shader::BUILTIN_TEXTURE_MAIN      },
	{ "love_VideoYChannel",    Shader::BUILTIN_TEXTURE_VIDEO_Y   },
	{ "love_VideoCbChannel",   Shader::BUILTIN_TEXTURE_VIDEO_CB  },
	{ "love_VideoCrChannel",   Shader::BUILTIN_TEXTURE_VIDEO_CR  },
	{ "love_UniformsPerDraw",  Shader::BUILTIN_UNIFORMS_PER_DRAW },
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
