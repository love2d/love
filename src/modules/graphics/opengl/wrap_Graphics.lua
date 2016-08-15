R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2016 LOVE Development Team

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

-- SHADERS

local GLSL = {}

GLSL.VERSION = "#version 120"
GLSL.VERSION_ES = "#version 100"

GLSL.SYNTAX = [[
#ifndef GL_ES
#define lowp
#define mediump
#define highp
#endif
#define number float
#define Image sampler2D
#define extern uniform
#define Texel texture2D
#pragma optionNV(strict on)]]

-- Uniforms shared by the vertex and pixel shader stages.
GLSL.UNIFORMS = [[
#ifdef GL_ES
// According to the GLSL ES 1.0 spec, uniform precision must match between stages,
// but we can't guarantee that highp is always supported in fragment shaders...
// We *really* don't want to use mediump for these in vertex shaders though.
#if defined(VERTEX) || defined(GL_FRAGMENT_PRECISION_HIGH)
#define LOVE_UNIFORM_PRECISION highp
#else
#define LOVE_UNIFORM_PRECISION mediump
#endif
uniform LOVE_UNIFORM_PRECISION mat4 TransformMatrix;
uniform LOVE_UNIFORM_PRECISION mat4 ProjectionMatrix;
uniform LOVE_UNIFORM_PRECISION mat4 TransformProjectionMatrix;
uniform LOVE_UNIFORM_PRECISION mat3 NormalMatrix;
#else
#define TransformMatrix gl_ModelViewMatrix
#define ProjectionMatrix gl_ProjectionMatrix
#define TransformProjectionMatrix gl_ModelViewProjectionMatrix
#define NormalMatrix gl_NormalMatrix
#endif
uniform mediump vec4 love_ScreenSize;]]

GLSL.FUNCTIONS = [[
float gammaToLinearPrecise(float c) {
	return c <= 0.04045 ? c * 0.077399380804954 : pow((c + 0.055) * 0.9478672985782, 2.4);
}
vec3 gammaToLinearPrecise(vec3 c) {
	bvec3 leq = lessThanEqual(c, vec3(0.04045));
	c.r = leq.r ? c.r * 0.077399380804954 : pow((c.r + 0.055) * 0.9478672985782, 2.4);
	c.g = leq.g ? c.g * 0.077399380804954 : pow((c.g + 0.055) * 0.9478672985782, 2.4);
	c.b = leq.b ? c.b * 0.077399380804954 : pow((c.b + 0.055) * 0.9478672985782, 2.4);
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

// pow(x, 2.2) isn't an amazing approximation, but at least it's efficient...
mediump float gammaToLinearFast(mediump float c) { return pow(max(c, 0.0), 2.2); }
mediump vec3 gammaToLinearFast(mediump vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
mediump vec4 gammaToLinearFast(mediump vec4 c) { return vec4(gammaToLinearFast(c.rgb), c.a); }
mediump float linearToGammaFast(mediump float c) { return pow(max(c, 0.0), 1.0 / 2.2); }
mediump vec3 linearToGammaFast(mediump vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }
mediump vec4 linearToGammaFast(mediump vec4 c) { return vec4(linearToGammaFast(c.rgb), c.a); }

#ifdef LOVE_PRECISE_GAMMA
#define gammaToLinear gammaToLinearPrecise
#define linearToGamma linearToGammaPrecise
#else
#define gammaToLinear gammaToLinearFast
#define linearToGamma linearToGammaFast
#endif

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
#define VERTEX
#define LOVE_PRECISE_GAMMA

attribute vec4 VertexPosition;
attribute vec4 VertexTexCoord;
attribute vec4 VertexColor;
attribute vec4 ConstantColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

#ifdef GL_ES
uniform mediump float love_PointSize;
#endif]],

	FUNCTIONS = "",

	FOOTER = [[
void main() {
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = gammaCorrectColor(VertexColor) * ConstantColor;
#ifdef GL_ES
	gl_PointSize = love_PointSize;
#endif
	gl_Position = position(TransformProjectionMatrix, VertexPosition);
}]],
}

GLSL.PIXEL = {
	HEADER = [[
#define PIXEL

#ifdef GL_ES
precision mediump float;
#endif

varying mediump vec4 VaryingTexCoord;
varying mediump vec4 VaryingColor;

#define love_Canvases gl_FragData

uniform sampler2D _tex0_;]],

	FUNCTIONS = [[
uniform sampler2D love_VideoYChannel;
uniform sampler2D love_VideoCbChannel;
uniform sampler2D love_VideoCrChannel;

vec4 VideoTexel(vec2 texcoords)
{
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

	FOOTER = [[
void main() {
	// fix crashing issue in OSX when _tex0_ is unused within effect()
	float dummy = Texel(_tex0_, vec2(.5)).r;

	// See Shader::checkSetScreenParams in Shader.cpp.
	vec2 pixelcoord = vec2(gl_FragCoord.x, (gl_FragCoord.y * love_ScreenSize.z) + love_ScreenSize.w);

	gl_FragColor = effect(VaryingColor, _tex0_, VaryingTexCoord.st, pixelcoord);
}]],

	FOOTER_MULTI_CANVAS = [[
void main() {
	// fix crashing issue in OSX when _tex0_ is unused within effect()
	float dummy = Texel(_tex0_, vec2(.5)).r;

	// See Shader::checkSetScreenParams in Shader.cpp.
	vec2 pixelcoord = vec2(gl_FragCoord.x, (gl_FragCoord.y * love_ScreenSize.z) + love_ScreenSize.w);

	effects(VaryingColor, _tex0_, VaryingTexCoord.st, pixelcoord);
}]],
}

local function createShaderStageCode(stage, code, lang, gammacorrect, multicanvas)
	stage = stage:upper()
	local lines = {
		lang == "glsles" and GLSL.VERSION_ES or GLSL.VERSION,
		GLSL.SYNTAX,
		gammacorrect and "#define LOVE_GAMMA_CORRECT 1" or "",
		GLSL[stage].HEADER,
		GLSL.UNIFORMS,
		GLSL.FUNCTIONS,
		GLSL[stage].FUNCTIONS,
		lang == "glsles" and "#line 1" or "#line 0",
		code,
		multicanvas and GLSL[stage].FOOTER_MULTI_CANVAS or GLSL[stage].FOOTER,
	}
	return table_concat(lines, "\n")
end

local function isVertexCode(code)
	return code:match("vec4%s+position%s*%(") ~= nil
end

local function isPixelCode(code)
	if code:match("vec4%s+effect%s*%(") then
		return true
	elseif code:match("void%s+effects%s*%(") then
		-- function for rendering to multiple canvases simultaneously
		return true, true
	else
		return false
	end
end

function love.graphics._shaderCodeToGLSL(arg1, arg2)
	local vertexcode, pixelcode
	local is_multicanvas = false -- whether pixel code has "effects" function instead of "effect"

	if arg1 then
		if isVertexCode(arg1) then
			vertexcode = arg1 -- first arg contains vertex shader code
		end

		local ispixel, isMultiCanvas = isPixelCode(arg1)
		if ispixel then
			pixelcode = arg1 -- first arg contains pixel shader code
			is_multicanvas = isMultiCanvas
		end
	end
	
	if arg2 then
		if isVertexCode(arg2) then
			vertexcode = arg2 -- second arg contains vertex shader code
		end

		local ispixel, isMultiCanvas = isPixelCode(arg2)
		if ispixel then
			pixelcode = arg2 -- second arg contains pixel shader code
			is_multicanvas = isMultiCanvas
		end
	end

	local lang = "glsl"
	if love.graphics.getRendererInfo() == "OpenGL ES" then
		lang = "glsles"
	end

	local gammacorrect = love.graphics.isGammaCorrect()

	if vertexcode then
		vertexcode = createShaderStageCode("VERTEX", vertexcode, lang, gammacorrect)
	end
	if pixelcode then
		pixelcode = createShaderStageCode("PIXEL", pixelcode, lang, gammacorrect, is_multicanvas)
	end

	return vertexcode, pixelcode
end

function love.graphics._transformGLSLErrorMessages(message)
	local shadertype = message:match("Cannot compile (%a+) shader code")
	if not shadertype then return message end
	local lines = {"Cannot compile "..shadertype.." shader code:"}
	for l in message:gmatch("[^\n]+") do
		-- nvidia compiler message:
		-- 0(<linenumber>) : error/warning [NUMBER]: <error message>
		local linenumber, what, message = l:match("^0%((%d+)%)%s*:%s*(%w+)[^:]+:%s*(.+)$")
		if not linenumber then
			-- ati compiler message:
			-- ERROR 0:<linenumber>: error/warning(#[NUMBER]) [ERRORNAME]: <errormessage>
			linenumber, what, message = l:match("^%w+: 0:(%d+):%s*(%w+)%([^%)]+%)%s*(.+)$")
			if not linenumber then
				-- OSX compiler message (?):
				-- ERROR: 0:<linenumber>: <errormessage>
				what, linenumber, message = l:match("^(%w+): %d+:(%d+): (.+)$")
			end
		end
		if linenumber and what and message then
			lines[#lines+1] = ("Line %d: %s: %s"):format(linenumber, what, message)
		end
	end
	-- did not match any known error messages
	if #lines == 1 then return message end
	return table_concat(lines, "\n")
end

local defaultcode = {
	vertex = [[
vec4 position(mat4 transform_proj, vec4 vertpos) {
	return transform_proj * vertpos;
}]],
	pixel = [[
vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {
	return Texel(tex, texcoord) * vcolor;
}]],
	videopixel = [[
vec4 effect(mediump vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {
	return VideoTexel(texcoord) * vcolor;
}]],
}

local defaults = {}
local defaults_gammacorrect = {}

for _, lang in ipairs{"glsl", "glsles"} do
	for _, gammacorrect in ipairs{false, true} do
		local t = gammacorrect and defaults_gammacorrect or defaults
		t[lang] = {
			vertex = createShaderStageCode("VERTEX", defaultcode.vertex, lang, gammacorrect),
			pixel = createShaderStageCode("PIXEL", defaultcode.pixel, lang, gammacorrect, false),
			videopixel = createShaderStageCode("PIXEL", defaultcode.videopixel, lang, gammacorrect, false),
		}
	end
end

love.graphics._setDefaultShaderCode(defaults, defaults_gammacorrect)

function love.graphics.newVideo(file, loadaudio)
	local video = love.graphics._newVideo(file)
	local source, success

	if loadaudio ~= false and love.audio then
		success, source = pcall(love.audio.newSource, video:getStream():getFilename())
	end
	if success then
		video:setSource(source)
	elseif loadaudio == true then
		if love.audio then
			error("Video had no audio track", 2)
		else
			error("love.audio was not loaded", 2)
		end
	else
		video:getStream():setSync()
	end

	return video
end

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
