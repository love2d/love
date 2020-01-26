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

#include "Shader.h"
#include "Graphics.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"
#include "libraries/spirv_cross/spirv_msl.hpp"

namespace love
{
namespace graphics
{
namespace metal
{

Shader::Shader(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel)
	: love::graphics::Shader(vertex, pixel)
{ @autoreleasepool {
	auto gfx = Graphics::getInstance();

	using namespace glslang;
	using namespace spirv_cross;

	// TODO: can this be done in ShaderStage (no linking)?

	glslang::TProgram program;

	if (vertex != nullptr)
		program.addShader((TShader *) vertex->getHandle());

	if (pixel != nullptr)
		program.addShader((TShader *) pixel->getHandle());

	if (!program.link(EShMsgDefault))
	{
		//err = "Cannot compile shader:\n\n" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
	}

	for (int i = 0; i < EShLangCount; i++)
	{
		auto intermediate = program.getIntermediate((EShLanguage)i);
		if (intermediate == nullptr)
			continue;

		spv::SpvBuildLogger logger;
		glslang::SpvOptions opt;
		opt.validate = true;

		std::vector<unsigned int> spirv;

		{
//			timer::ScopedTimer("shader stage");
			GlslangToSpv(*intermediate, spirv, &logger, &opt);
		}

		std::string msgs = logger.getAllMessages();
//		printf("spirv length: %ld, messages:\n%s\n", spirv.size(), msgs.c_str());

		// Compile to GLSL, ready to give to GL driver.
		try
		{

//			printf("GLSL INPUT SOURCE:\n\n%s\n\n", pixel->getSource().c_str());

			CompilerMSL msl(std::move(spirv));

			CompilerMSL::Options options;

#ifdef LOVE_IOS
			options.platform = CompilerMSL::Options::iOS;
#else
			options.platform = CompilerMSL::Options::macOS;
#endif

			msl.set_msl_options(options);

			std::string source = msl.compile();
//			printf("MSL SOURCE:\n\n%s\n\n", source.c_str());

			NSString *nssource = [[NSString alloc] initWithBytes:source.c_str()
														  length:source.length()
														encoding:NSUTF8StringEncoding];

			NSError *err = nil;
			id<MTLLibrary> library = [gfx->device newLibraryWithSource:nssource options:nil error:&err];

		}
		catch (std::exception &e)
		{
			printf("Error parsing SPIR-V shader source: %s\n", e.what());
		}
	}
}}

Shader::~Shader()
{ @autoreleasepool {

}}

} // metal
} // graphics
} // love
