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

do
	local table_concat = table.concat

	-- SHADERS

	local GLSL_VERSION = "#version 120"
	
	local GLSL_SYNTAX = [[
#define lowp
#define mediump
#define highp
#define number float
#define Image sampler2D
#define extern uniform
#define Texel texture2D
#pragma optionNV(strict on)]]

	local GLSL_UNIFORMS = [[
#define TransformMatrix gl_ModelViewMatrix
#define ProjectionMatrix gl_ProjectionMatrix
#define TransformProjectionMatrix gl_ModelViewProjectionMatrix
#define NormalMatrix gl_NormalMatrix
uniform sampler2D _tex0_;
uniform vec4 love_ScreenSize;]]

	local GLSL_VERTEX = {
		HEADER = [[
#define VERTEX

#define VertexPosition gl_Vertex
#define VertexTexCoord gl_MultiTexCoord0
#define VertexColor gl_Color

#define VaryingTexCoord gl_TexCoord[0]
#define VaryingColor gl_FrontColor

// #if defined(GL_ARB_draw_instanced)
//	#extension GL_ARB_draw_instanced : enable
//	#define love_InstanceID gl_InstanceIDARB
// #else
//	attribute float love_PseudoInstanceID;
//	int love_InstanceID = int(love_PseudoInstanceID);
// #endif
]],

		FOOTER = [[
void main() {
	VaryingTexCoord = VertexTexCoord;
	VaryingColor = VertexColor;
	gl_Position = position(TransformProjectionMatrix, VertexPosition);
}]],
	}

	local GLSL_PIXEL = {
		HEADER = [[
#define PIXEL

#define VaryingTexCoord gl_TexCoord[0]
#define VaryingColor gl_Color

#define love_Canvases gl_FragData]],

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

	local function createVertexCode(vertexcode)
		local vertexcodes = {
			GLSL_VERSION,
			GLSL_SYNTAX, GLSL_VERTEX.HEADER, GLSL_UNIFORMS,
			"#line 0",
			vertexcode,
			GLSL_VERTEX.FOOTER,
		}
		return table_concat(vertexcodes, "\n")
	end

	local function createPixelCode(pixelcode, is_multicanvas)
		local pixelcodes = {
			GLSL_VERSION,
			GLSL_SYNTAX, GLSL_PIXEL.HEADER, GLSL_UNIFORMS,
			"#line 0",
			pixelcode,
			is_multicanvas and GLSL_PIXEL.FOOTER_MULTI_CANVAS or GLSL_PIXEL.FOOTER,
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
			vertexcode = createVertexCode(vertexcode)
		end
		if pixelcode then
			pixelcode = createPixelCode(pixelcode, is_multicanvas)
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
end
