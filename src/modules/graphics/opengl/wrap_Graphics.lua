R"luastring"--(
-- DO NOT REMOVE THE ABOVE LINE. It is used to load this file as a C++ string.
-- There is a matching delimiter at the bottom of the file.

--[[
Copyright (c) 2006-2015 LOVE Development Team

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
#else
#define TransformMatrix gl_ModelViewMatrix
#define ProjectionMatrix gl_ProjectionMatrix
#define TransformProjectionMatrix gl_ModelViewProjectionMatrix
#endif
uniform mediump vec4 love_ScreenSize;]]

GLSL.VERTEX = {
	HEADER = [[
#define VERTEX

attribute vec4 VertexPosition;
attribute vec4 VertexTexCoord;
attribute vec4 VertexColor;

varying vec4 VaryingTexCoord;
varying vec4 VaryingColor;

#ifdef GL_ES
uniform mediump float love_PointSize;
#endif]],

	FOOTER = [[
void main() {
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = VertexColor;
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
varying lowp vec4 VaryingColor;

#define love_Canvases gl_FragData

uniform sampler2D _tex0_;]],

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

local function createVertexCode(vertexcode, lang)
	local vertexcodes = {
		lang == "glsles" and GLSL.VERSION_ES or GLSL.VERSION,
		GLSL.SYNTAX, GLSL.VERTEX.HEADER, GLSL.UNIFORMS,
		lang == "glsles" and "#line 1" or "#line 0",
		vertexcode,
		GLSL.VERTEX.FOOTER,
	}
	return table_concat(vertexcodes, "\n")
end

local function createPixelCode(pixelcode, is_multicanvas, lang)
	local pixelcodes = {
		lang == "glsles" and GLSL.VERSION_ES or GLSL.VERSION,
		GLSL.SYNTAX, GLSL.PIXEL.HEADER, GLSL.UNIFORMS,
		lang == "glsles" and "#line 1" or "#line 0",
		pixelcode,
		is_multicanvas and GLSL.PIXEL.FOOTER_MULTI_CANVAS or GLSL.PIXEL.FOOTER,
	}
	return table_concat(pixelcodes, "\n")
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

	local lang = "glsl"
	if (love.graphics.getRendererInfo()) == "OpenGL ES" then
		lang = "glsles"
	end

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

	if vertexcode then
		vertexcode = createVertexCode(vertexcode, lang)
	end
	if pixelcode then
		pixelcode = createPixelCode(pixelcode, is_multicanvas, lang)
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
vec4 effect(lowp vec4 vcolor, Image tex, vec2 texcoord, vec2 pixcoord) {
	return Texel(tex, texcoord) * vcolor;
}]]
}

local defaults = {
	opengl = {
		createVertexCode(defaultcode.vertex, "glsl"),
		createPixelCode(defaultcode.pixel, false, "glsl"),
	},
	opengles = {
		createVertexCode(defaultcode.vertex, "glsles"),
		createPixelCode(defaultcode.pixel, false, "glsles"),
	},
}

love.graphics._setDefaultShaderCode(defaults)

-- DO NOT REMOVE THE NEXT LINE. It is used to load this file as a C++ string.
--)luastring"--"
