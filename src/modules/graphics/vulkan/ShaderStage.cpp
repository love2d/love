#include "ShaderStage.h"

#include "Graphics.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"

#include <iostream>
#include <fstream>

#include <cstdio>


namespace love {
namespace graphics {
namespace vulkan {
ShaderStage::ShaderStage(love::graphics::Graphics* gfx, ShaderStageType stage, const std::string& glsl, bool gles, const std::string& cachekey)
	: love::graphics::ShaderStage(gfx, stage, glsl, gles, cachekey) {
	// the compilation is done in Shader.
}
} // love
} // graphics
} // vulkan
