#ifndef LOVE_GRAPHICS_VULKAN_SHADERSTAGE_H
#define LOVE_GRAPHICS_VULKAN_SHADERSTAGE_H

#include "graphics/ShaderStage.h"
#include "modules/graphics/Graphics.h"
#include <vulkan/vulkan.h>

namespace love {
	namespace graphics {
		namespace vulkan {
			class ShaderStage final : public graphics::ShaderStage, public Volatile {
			public:
				ShaderStage(love::graphics::Graphics* gfx, ShaderStageType stage, const std::string& glsl, bool gles, const std::string& cachekey);
				virtual ~ShaderStage();

				virtual bool loadVolatile() override;

				virtual void unloadVolatile() override;

				VkShaderModule getShaderModule() const {
					return shaderModule;
				}

				ptrdiff_t getHandle() const {
					return 0;
				}

			private:
				VkShaderModule shaderModule = VK_NULL_HANDLE;
				VkDevice device;
				love::graphics::Graphics* gfx;
			};
		}
	}
}

#endif
