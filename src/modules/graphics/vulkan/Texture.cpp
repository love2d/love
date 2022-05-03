#include "Texture.h"

namespace love {
	namespace graphics {
		namespace vulkan {
			Texture::Texture(love::graphics::Graphics* gfx, const Settings& settings, const Slices* data)
				: love::graphics::Texture(gfx, settings, data) {
			}
		}
	}
}