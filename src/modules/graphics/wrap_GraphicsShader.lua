R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2023 LOVE Development Team

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
--]]

local table_concat = table.concat
local ipairs = ipairs

local GLSL = {}

GLSL.VERSION = { -- index using [target][gles]
	glsl1 = {[false]="#version 120",      [true]="#version 100"},
	glsl3 = {[false]="#version 330 core", [true]="#version 300 es"},
}

GLSL.SYNTAX = [[
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
]]

-- Uniforms shared by the vertex and pixel shader stages.
GLSL.UNIFORMS = [[
// According to the GLSL ES 1.0 spec, uniform precision must match between stages,
// but we can't guarantee that highp is always supported in fragment shaders...
// We *really* don't want to use mediump for these in vertex shaders though.
uniform LOVE_HIGHP_OR_MEDIUMP mat4 ViewSpaceFromLocal;
uniform LOVE_HIGHP_OR_MEDIUMP mat4 ClipSpaceFromView;
uniform LOVE_HIGHP_OR_MEDIUMP mat4 ClipSpaceFromLocal;
uniform LOVE_HIGHP_OR_MEDIUMP mat3 ViewNormalFromLocal;
uniform LOVE_HIGHP_OR_MEDIUMP vec4 love_ScreenSize;

// Compatibility
#define TransformMatrix ViewSpaceFromLocal
#define ProjectionMatrix ClipSpaceFromView
#define TransformProjectionMatrix ClipSpaceFromLocal
#define NormalMatrix ViewNormalFromLocal
]]

GLSL.FUNCTIONS = [[
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
#endif]]

GLSL.VERTEX = {
	HEADER = [[
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
#endif]],

	FUNCTIONS = [[
void setPointSize() {
#ifdef GL_ES
	gl_PointSize = love_PointSize;
#endif
}]],

	MAIN = [[
attribute vec4 VertexPosition;
attribute vec4 VertexTexCoord;
attribute vec4 VertexColor;
attribute vec4 ConstantColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition);

void main() {
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;
	setPointSize();
	love_Position = position(ClipSpaceFromLocal, VertexPosition);
}]],
}

GLSL.PIXEL = {
	HEADER = [[
#ifdef GL_ES
	precision mediump float;
#endif

#define love_MaxCanvases gl_MaxDrawBuffers

#if __VERSION__ >= 130
	#define varying in
	// Some drivers seem to make the pixel shader do more work when multiple
	// pixel shader outputs are defined, even when only one is actually used.
	// TODO: We should use reflection or something instead of this, to determine
	// how many outputs are actually used in the shader code.
	#ifdef LOVE_MULTI_CANVAS
		layout(location = 0) out vec4 love_Canvases[love_MaxCanvases];
		#define love_PixelColor love_Canvases[0]
	#else
		layout(location = 0) out vec4 love_PixelColor;
	#endif
#else
	#ifdef LOVE_MULTI_CANVAS
		#define love_Canvases gl_FragData
	#endif
	#define love_PixelColor gl_FragColor
#endif

// See Shader::updateScreenParams in Shader.cpp.
#define love_PixelCoord (vec2(gl_FragCoord.x, (gl_FragCoord.y * love_ScreenSize.z) + love_ScreenSize.w))]],

	FUNCTIONS = [[
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
}]],

	MAIN = [[
uniform sampler2D MainTex;
varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

vec4 effect(vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord);

void main() {
	love_PixelColor = effect(VaryingColor, MainTex, VaryingTexCoord.st, love_PixelCoord);
}]],

	MAIN_CUSTOM = [[
varying LOVE_HIGHP_OR_MEDIUMP vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

void effect();

void main() {
	effect();
}]],
}

local function getLanguageTarget(code)
	if not code then return nil end
	return (code:match("^%s*#pragma language (%w+)")) or "glsl1"
end

local function createShaderStageCode(stage, code, lang, gles, glsl1on3, gammacorrect, custom, multicanvas)
	stage = stage:upper()
	local lines = {
		GLSL.VERSION[lang][gles],
		"#define " ..stage .. " " .. stage,
		glsl1on3 and "#define LOVE_GLSL1_ON_GLSL3 1" or "",
		gammacorrect and "#define LOVE_GAMMA_CORRECT 1" or "",
		multicanvas and "#define LOVE_MULTI_CANVAS 1" or "",
		GLSL.SYNTAX,
		GLSL[stage].HEADER,
		GLSL.UNIFORMS,
		GLSL.FUNCTIONS,
		GLSL[stage].FUNCTIONS,
		custom and GLSL[stage].MAIN_CUSTOM or GLSL[stage].MAIN,
		((lang == "glsl1" or glsl1on3) and not gles) and "#line 0" or "#line 1",
		code,
	}
	return table_concat(lines, "\n")
end

local function isVertexCode(code)
	return code:match("vec4%s+position%s*%(") ~= nil
end

local function isPixelCode(code)
	if code:match("vec4%s+effect%s*%(") then
		return true
	elseif code:match("void%s+effect%s*%(") then -- custom effect function
		local multicanvas = code:match("love_Canvases") ~= nil
		return true, true, multicanvas
	else
		return false
	end
end

function love.graphics._shaderCodeToGLSL(gles, arg1, arg2)
	local vertexcode, pixelcode
	local is_custompixel = false -- whether pixel code has "effects" function instead of "effect"
	local is_multicanvas = false

	if arg1 then
		if isVertexCode(arg1) then
			vertexcode = arg1 -- first arg contains vertex shader code
		end

		local ispixel, isCustomPixel, isMultiCanvas = isPixelCode(arg1)
		if ispixel then
			pixelcode = arg1 -- first arg contains pixel shader code
			is_custompixel, is_multicanvas = isCustomPixel, isMultiCanvas
		end
	end
	
	if arg2 then
		if isVertexCode(arg2) then
			vertexcode = arg2 -- second arg contains vertex shader code
		end

		local ispixel, isCustomPixel, isMultiCanvas = isPixelCode(arg2)
		if ispixel then
			pixelcode = arg2 -- second arg contains pixel shader code
			is_custompixel, is_multicanvas = isCustomPixel, isMultiCanvas
		end
	end

	local supportsGLSL3 = love.graphics.getSupported().glsl3
	local gammacorrect = love.graphics.isGammaCorrect()

	local targetlang = getLanguageTarget(pixelcode or vertexcode)
	if getLanguageTarget(vertexcode or pixelcode) ~= targetlang then
		error("vertex and pixel shader languages must match", 2)
	end

	if targetlang == "glsl3" and not supportsGLSL3 then
		error("GLSL 3 shaders are not supported on this system!", 2)
	end

	if targetlang ~= nil and not GLSL.VERSION[targetlang] then
		error("Invalid shader language: " .. targetlang, 2)
	end

	local lang = targetlang or "glsl1"
	local glsl1on3 = false
	if lang == "glsl1" and supportsGLSL3 then
		lang = "glsl3"
		glsl1on3 = true
	end

	if vertexcode then
		vertexcode = createShaderStageCode("VERTEX", vertexcode, lang, gles, glsl1on3, gammacorrect)
	end
	if pixelcode then
		pixelcode = createShaderStageCode("PIXEL", pixelcode, lang, gles, glsl1on3, gammacorrect, is_custompixel, is_multicanvas)
	end

	return vertexcode, pixelcode
end

function love.graphics._transformGLSLErrorMessages(message)
	local shadertype = message:match("Cannot compile (%a+) shader code")
	local compiling = shadertype ~= nil
	if not shadertype then
		shadertype = message:match("Error validating (%a+) shader")
	end
	if not shadertype then return message end
	local lines = {}
	local prefix = compiling and "Cannot compile " or "Error validating "
	lines[#lines+1] = prefix..shadertype.." shader code:"
	for l in message:gmatch("[^\n]+") do
		-- nvidia: 0(<linenumber>) : error/warning [NUMBER]: <error message>
		local linenumber, what, message = l:match("^0%((%d+)%)%s*:%s*(%w+)[^:]+:%s*(.+)$")
		if not linenumber then
			-- AMD: ERROR 0:<linenumber>: error/warning(#[NUMBER]) [ERRORNAME]: <errormessage>
			linenumber, what, message = l:match("^%w+: 0:(%d+):%s*(%w+)%([^%)]+%)%s*(.+)$")
		end
		if not linenumber then
			-- macOS (?): ERROR: 0:<linenumber>: <errormessage>
			what, linenumber, message = l:match("^(%w+): %d+:(%d+): (.+)$")
		end
		if not linenumber and l:match("^ERROR:") then
			what = l
		end
		if linenumber and what and message then
			lines[#lines+1] = ("Line %d: %s: %s"):format(linenumber, what, message)
		elseif what then
			lines[#lines+1] = what
		end
	end
	-- did not match any known error messages
	if #lines == 1 then return message end
	return table_concat(lines, "\n")
end

local defaultcode = {
	vertex = [[
vec4 position(mat4 clipSpaceFromLocal, vec4 localPosition) {
	return clipSpaceFromLocal * localPosition;
}]],
	pixel = [[
vec4 effect(vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {
	return Texel(tex, texcoord) * vcolor;
}]],
	videopixel = [[
void effect() {
	love_PixelColor = VideoTexel(VaryingTexCoord.xy) * VaryingColor;
}]],
	arraypixel = [[
uniform ArrayImage MainTex;
void effect() {
	love_PixelColor = Texel(MainTex, VaryingTexCoord.xyz) * VaryingColor;
}]],
}

local defaults = {}
local defaults_gammacorrect = {}

local langs = {
	glsl1 = {target="glsl1", gles=false},
	essl1 = {target="glsl1", gles=true},
	glsl3 = {target="glsl3", gles=false},
	essl3 = {target="glsl3", gles=true},
}

for lang, info in pairs(langs) do
	for _, gammacorrect in ipairs{false, true} do
		local t = gammacorrect and defaults_gammacorrect or defaults
		t[lang] = {
			vertex = createShaderStageCode("VERTEX", defaultcode.vertex, info.target, info.gles, false, gammacorrect),
			pixel = createShaderStageCode("PIXEL", defaultcode.pixel, info.target, info.gles, false, gammacorrect, false),
			videopixel = createShaderStageCode("PIXEL", defaultcode.videopixel, info.target, info.gles, false, gammacorrect, true),
			arraypixel = createShaderStageCode("PIXEL", defaultcode.arraypixel, info.target, info.gles, false, gammacorrect, true),
		}
	end
end

love.graphics._setDefaultShaderCode(defaults, defaults_gammacorrect)

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
