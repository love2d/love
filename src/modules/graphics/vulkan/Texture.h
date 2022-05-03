#include "graphics/Texture.h"


namespace love {
	namespace graphics {
		namespace vulkan {
			class Texture : public graphics::Texture {
			public:
				Texture(love::graphics::Graphics* gfx, const Settings& settings, const Slices* data);

				void copyFromBuffer(Buffer* source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect& rect) override {};
				void copyToBuffer(Buffer* dest, int slice, int mipmap, const Rect& rect, size_t destoffset, int destwidth, size_t size) override {};

				ptrdiff_t getRenderTargetHandle() const override {};
				ptrdiff_t getSamplerHandle() const override {};

				void uploadByteData(PixelFormat pixelformat, const void* data, size_t size, int level, int slice, const Rect& r)  override {};

				void generateMipmapsInternal()  override {};
				void readbackImageData(love::image::ImageData* imagedata, int slice, int mipmap, const Rect& rect)  override {};
			};
		}
	}
}