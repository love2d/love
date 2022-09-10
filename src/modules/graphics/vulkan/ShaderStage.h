#pragma once

#include "graphics/ShaderStage.h"
#include "modules/graphics/Graphics.h"

#include "VulkanWrapper.h"

namespace love
{
namespace graphics
{
namespace vulkan
{

class ShaderStage final : public graphics::ShaderStage
{
public:
	ShaderStage(love::graphics::Graphics *gfx, ShaderStageType stage, const std::string &glsl, bool gles, const std::string &cachekey);

	ptrdiff_t getHandle() const override;
};

}
}
}
