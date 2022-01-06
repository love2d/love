#include "ShaderStage.h"

#include "Graphics.h"

#include <libraries/glslang/glslang/Public/ShaderLang.h>
#include <libraries/glslang/SPIRV/GlslangToSpv.h>


namespace love {
	namespace graphics {
		namespace vulkan {
			// TODO: Use love.graphics to determine actual limits?
			static const TBuiltInResource defaultTBuiltInResource = {
				/* .MaxLights = */ 32,
				/* .MaxClipPlanes = */ 6,
				/* .MaxTextureUnits = */ 32,
				/* .MaxTextureCoords = */ 32,
				/* .MaxVertexAttribs = */ 64,
				/* .MaxVertexUniformComponents = */ 16384,
				/* .MaxVaryingFloats = */ 128,
				/* .MaxVertexTextureImageUnits = */ 32,
				/* .MaxCombinedTextureImageUnits = */ 80,
				/* .MaxTextureImageUnits = */ 32,
				/* .MaxFragmentUniformComponents = */ 16384,
				/* .MaxDrawBuffers = */ 8,
				/* .MaxVertexUniformVectors = */ 4096,
				/* .MaxVaryingVectors = */ 32,
				/* .MaxFragmentUniformVectors = */ 4096,
				/* .MaxVertexOutputVectors = */ 32,
				/* .MaxFragmentInputVectors = */ 31,
				/* .MinProgramTexelOffset = */ -8,
				/* .MaxProgramTexelOffset = */ 7,
				/* .MaxClipDistances = */ 8,
				/* .MaxComputeWorkGroupCountX = */ 65535,
				/* .MaxComputeWorkGroupCountY = */ 65535,
				/* .MaxComputeWorkGroupCountZ = */ 65535,
				/* .MaxComputeWorkGroupSizeX = */ 1024,
				/* .MaxComputeWorkGroupSizeY = */ 1024,
				/* .MaxComputeWorkGroupSizeZ = */ 64,
				/* .MaxComputeUniformComponents = */ 1024,
				/* .MaxComputeTextureImageUnits = */ 32,
				/* .MaxComputeImageUniforms = */ 16,
				/* .MaxComputeAtomicCounters = */ 4096,
				/* .MaxComputeAtomicCounterBuffers = */ 8,
				/* .MaxVaryingComponents = */ 128,
				/* .MaxVertexOutputComponents = */ 128,
				/* .MaxGeometryInputComponents = */ 128,
				/* .MaxGeometryOutputComponents = */ 128,
				/* .MaxFragmentInputComponents = */ 128,
				/* .MaxImageUnits = */ 192,
				/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 144,
				/* .MaxCombinedShaderOutputResources = */ 144,
				/* .MaxImageSamples = */ 32,
				/* .MaxVertexImageUniforms = */ 16,
				/* .MaxTessControlImageUniforms = */ 16,
				/* .MaxTessEvaluationImageUniforms = */ 16,
				/* .MaxGeometryImageUniforms = */ 16,
				/* .MaxFragmentImageUniforms = */ 16,
				/* .MaxCombinedImageUniforms = */ 80,
				/* .MaxGeometryTextureImageUnits = */ 16,
				/* .MaxGeometryOutputVertices = */ 256,
				/* .MaxGeometryTotalOutputComponents = */ 1024,
				/* .MaxGeometryUniformComponents = */ 1024,
				/* .MaxGeometryVaryingComponents = */ 64,
				/* .MaxTessControlInputComponents = */ 128,
				/* .MaxTessControlOutputComponents = */ 128,
				/* .MaxTessControlTextureImageUnits = */ 16,
				/* .MaxTessControlUniformComponents = */ 1024,
				/* .MaxTessControlTotalOutputComponents = */ 4096,
				/* .MaxTessEvaluationInputComponents = */ 128,
				/* .MaxTessEvaluationOutputComponents = */ 128,
				/* .MaxTessEvaluationTextureImageUnits = */ 16,
				/* .MaxTessEvaluationUniformComponents = */ 1024,
				/* .MaxTessPatchComponents = */ 120,
				/* .MaxPatchVertices = */ 32,
				/* .MaxTessGenLevel = */ 64,
				/* .MaxViewports = */ 16,
				/* .MaxVertexAtomicCounters = */ 4096,
				/* .MaxTessControlAtomicCounters = */ 4096,
				/* .MaxTessEvaluationAtomicCounters = */ 4096,
				/* .MaxGeometryAtomicCounters = */ 4096,
				/* .MaxFragmentAtomicCounters = */ 4096,
				/* .MaxCombinedAtomicCounters = */ 4096,
				/* .MaxAtomicCounterBindings = */ 8,
				/* .MaxVertexAtomicCounterBuffers = */ 8,
				/* .MaxTessControlAtomicCounterBuffers = */ 8,
				/* .MaxTessEvaluationAtomicCounterBuffers = */ 8,
				/* .MaxGeometryAtomicCounterBuffers = */ 8,
				/* .MaxFragmentAtomicCounterBuffers = */ 8,
				/* .MaxCombinedAtomicCounterBuffers = */ 8,
				/* .MaxAtomicCounterBufferSize = */ 16384,
				/* .MaxTransformFeedbackBuffers = */ 4,
				/* .MaxTransformFeedbackInterleavedComponents = */ 64,
				/* .MaxCullDistances = */ 8,
				/* .MaxCombinedClipAndCullDistances = */ 8,
				/* .MaxSamples = */ 32,
				/* .maxMeshOutputVerticesNV = */ 256,
				/* .maxMeshOutputPrimitivesNV = */ 512,
				/* .maxMeshWorkGroupSizeX_NV = */ 32,
				/* .maxMeshWorkGroupSizeY_NV = */ 1,
				/* .maxMeshWorkGroupSizeZ_NV = */ 1,
				/* .maxTaskWorkGroupSizeX_NV = */ 32,
				/* .maxTaskWorkGroupSizeY_NV = */ 1,
				/* .maxTaskWorkGroupSizeZ_NV = */ 1,
				/* .maxMeshViewCountNV = */ 4,
				/* .maxDualSourceDrawBuffersEXT = */ 1,
				/* .limits = */{
					/* .nonInductiveForLoops = */ 1,
					/* .whileLoops = */ 1,
					/* .doWhileLoops = */ 1,
					/* .generalUniformIndexing = */ 1,
					/* .generalAttributeMatrixVectorIndexing = */ 1,
					/* .generalVaryingIndexing = */ 1,
					/* .generalSamplerIndexing = */ 1,
					/* .generalVariableIndexing = */ 1,
					/* .generalConstantMatrixVectorIndexing = */ 1,
				}
			};

			static EShLanguage getShaderStage(ShaderStageType stage) {
				switch (stage) {
				case SHADERSTAGE_VERTEX: return EShLangVertex;
				case SHADERSTAGE_PIXEL: return EShLangFragment;
				case SHADERSTAGE_COMPUTE: return EShLangCompute;
				case SHADERSTAGE_MAX_ENUM: return EShLangCount;
				}
				return EShLangCount;
			}

			ShaderStage::ShaderStage(love::graphics::Graphics* gfx, ShaderStageType stage, const std::string& glsl, bool gles, const std::string& cachekey)
				: love::graphics::ShaderStage(gfx, stage, glsl, gles, cachekey) {
				if (false) {
					using namespace glslang;

					auto shaderStage = getShaderStage(stage);

					TShader* shader = new TShader(shaderStage);
					shader->setEnvInput(EShSourceGlsl, shaderStage, EShClientVulkan, 450);
					shader->setEnvClient(EShClientVulkan, EShTargetVulkan_1_2);
					shader->setEnvTarget(EShTargetSpv, EShTargetSpv_1_5);
					shader->setAutoMapLocations(true);
					shader->setAutoMapBindings(true);
					shader->setEnvInputVulkanRulesRelaxed();
					shader->setGlobalUniformBinding(0);
					shader->setGlobalUniformSet(0);

					const std::string& source = glsl;
					const char* csrc = source.c_str();
					int srclen = (int)source.length();
					shader->setStringsWithLengths(&csrc, &srclen, 1);

					int defaultversion = 450;
					EProfile defaultprofile = ECoreProfile;
					bool forcedefault = false;
					bool forwardcompat = true;

					if (!shader->parse(&defaultTBuiltInResource, defaultversion, defaultprofile, forcedefault, forwardcompat, EShMsgSuppressWarnings)) {
						const char* stagename = "unknown";
						ShaderStage::getConstant(stage, stagename);

						std::string err = "Error parsing " + std::string(stagename) + " shader:\n\n"
							+ std::string(shader->getInfoLog()) + "\n"
							+ std::string(shader->getInfoDebugLog());

						delete shader;

						throw love::Exception("%s", err.c_str());
					}

					auto intermediate = shader->getIntermediate();
					std::vector<unsigned int> code;
					GlslangToSpv(*intermediate, code);

					VkShaderModuleCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					createInfo.codeSize = code.size();
					createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

					Graphics* vkGfx = (Graphics*)gfx;
					device = vkGfx->getDevice();

					if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
						throw love::Exception("failed to create shader module");
					}
				}

			}

			ShaderStage::~ShaderStage() {
				if (false) 
					vkDestroyShaderModule(device, shaderModule, nullptr);
			}
		}
	}
}
