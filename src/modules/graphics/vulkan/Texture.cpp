#include "Texture.h"

namespace love {
	namespace graphics {
		namespace vulkan {
			Texture::Texture(love::graphics::Graphics* gfx, const Settings& settings, const Slices* data)
				: love::graphics::Texture(gfx, settings, data) {
			}

			void Texture::uploadByteData(PixelFormat pixelformat, const void* data, size_t size, int level, int slice, const Rect& r) {
				std::cout << "Texture::uploadByteData ";
			}
		}
	}
}