#include "graphics/Texture.h"
#include <iostream>


namespace love {
	namespace graphics {
		namespace vulkan {
			class Texture : public graphics::Texture {
			public:
				Texture(love::graphics::Graphics* gfx, const Settings& settings, const Slices* data);

				void copyFromBuffer(graphics::Buffer* source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect& rect) override { std::cout << "Texture::copyFromBuffer "; };
				void copyToBuffer(graphics::Buffer* dest, int slice, int mipmap, const Rect& rect, size_t destoffset, int destwidth, size_t size) override { std::cout << "Texture::copyToBuffer "; };

				ptrdiff_t getRenderTargetHandle() const override { std::cout << "Texture::getRenderTargetHandle "; return (ptrdiff_t)0; };
				ptrdiff_t getSamplerHandle() const override { std::cout << "Texture::getSamplerHandle "; return (ptrdiff_t)0; };

				void uploadByteData(PixelFormat pixelformat, const void* data, size_t size, int level, int slice, const Rect& r)  override;

				void generateMipmapsInternal()  override { std::cout << "Texture::generateMipmapsInternal "; };
				void readbackImageData(love::image::ImageData* imagedata, int slice, int mipmap, const Rect& rect)  override { std::cout << "Texture::readbackImageData "; };

				int getMSAA() const override { std::cout << "Texture::getMSAA "; return 0; };
				ptrdiff_t getHandle() const override { std::cout << "Texture::getHandle "; return (ptrdiff_t)0; }
			};
		}
	}
}