/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#pragma once

#include "libraries/xxHash/xxhash.h"
#include "common/int.h"
#include "graphics/Shader.h"
#include "graphics/Graphics.h"
#include "graphics/renderstate.h"
#include "graphics/vertex.h"
#include "Metal.h"

#import <Metal/MTLRenderPipeline.h>

#include <unordered_map>
#include <map>
#include <string>

namespace glslang
{
class TProgram;
}

namespace spirv_cross
{
class CompilerMSL;
struct SPIRType;
struct Resource;
}

namespace love
{
namespace graphics
{
namespace metal
{

static const int DEFAULT_VERTEX_BUFFER_BINDING = 1;

class Shader final : public love::graphics::Shader
{
public:

	struct RenderPipelineKey
	{
		VertexAttributesID vertexAttributesID;
		uint32 blendStateKey;
		uint64 colorRenderTargetFormats;
		uint32 depthStencilFormat;
		ColorChannelMask colorChannelMask;
		uint8 msaa;

		RenderPipelineKey()
		{
			memset(this, 0, sizeof(RenderPipelineKey));
		}

		bool operator == (const RenderPipelineKey &other) const
		{
			return memcmp(this, &other, sizeof(RenderPipelineKey)) == 0;
		}
	};

	struct TextureBinding
	{
		id<MTLTexture> texture;

		// Indirect sampler ref in case Texture's sampler state changes.
		Texture *samplerTexture;

		bool isMainTexture;
		Access access;

		uint8 textureStages[SHADERSTAGE_MAX_ENUM];
		uint8 samplerStages[SHADERSTAGE_MAX_ENUM];
	};

	struct BufferBinding
	{
		id<MTLBuffer> buffer;
		uint8 stages[SHADERSTAGE_MAX_ENUM];
		Access access;
	};

	Shader(id<MTLDevice> device, StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const CompileOptions &options);
	virtual ~Shader();

	// Implements Shader.
	void attach() override;
	std::string getWarnings() const override { return ""; }
	int getVertexAttributeIndex(const std::string &name) override;
	const UniformInfo *getUniformInfo(BuiltinUniform builtin) const override;
	void updateUniform(const UniformInfo *info, int count) override;
	ptrdiff_t getHandle() const override { return 0; }

	id<MTLRenderPipelineState> getCachedRenderPipeline(Graphics *gfx, const RenderPipelineKey &key);
	id<MTLComputePipelineState> getComputePipeline() const { return computePipeline; }

	static int getUniformBufferBinding();
	int getFirstVertexBufferBinding() const { return firstVertexBufferBinding; }

	const std::vector<TextureBinding> &getTextureBindings() const { return textureBindings; }
	const std::vector<BufferBinding> &getBufferBindings() const { return bufferBindings; }

	uint8 *getLocalUniformBufferData() { return localUniformBufferData; }
	size_t getLocalUniformBufferSize() const { return localUniformBufferSize; }
	size_t getBuiltinUniformDataOffset() const { return builtinUniformDataOffset; }

private:

	struct AttributeInfo
	{
		int index;
		DataBaseType baseType;
	};

	struct RenderPipelineHasher
	{
		size_t operator() (const RenderPipelineKey &key) const
		{
			return XXH32(&key, sizeof(RenderPipelineKey), 0);
		}
	};

	void buildLocalUniforms(const spirv_cross::CompilerMSL &msl, const spirv_cross::SPIRType &type, size_t baseoffset, const std::string &basename);
	void compileFromGLSLang(id<MTLDevice> device, const glslang::TProgram &program);

	void applyTexture(const UniformInfo *info, int i, love::graphics::Texture *texture, UniformType basetype, bool isdefault) override;
	void applyBuffer(const UniformInfo *info, int i, love::graphics::Buffer *buffer, UniformType basetype, bool isdefault) override;

	id<MTLFunction> functions[SHADERSTAGE_MAX_ENUM];

	UniformInfo *builtinUniformInfo[BUILTIN_MAX_ENUM];

	uint8 *localUniformStagingData;
	uint8 *localUniformBufferData;
	size_t localUniformBufferSize;
	size_t builtinUniformDataOffset;

	int firstVertexBufferBinding;

	std::map<std::string, AttributeInfo> attributes;

	std::vector<TextureBinding> textureBindings;
	std::vector<BufferBinding> bufferBindings;

	std::unordered_map<RenderPipelineKey, const void *, RenderPipelineHasher> cachedRenderPipelines;
	id<MTLComputePipelineState> computePipeline;

}; // Metal

} // metal
} // graphics
} // love
