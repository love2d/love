#include "Shader.h"
#include "Graphics.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"
#include "libraries/spirv_cross/spirv_cross.hpp"

#include <vector>

namespace love {
	namespace graphics {
		namespace vulkan {
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
				/* .limits = */ {
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

			static VkShaderStageFlagBits getStageBit(ShaderStageType type) {
				switch (type) {
				case SHADERSTAGE_VERTEX:
					return VK_SHADER_STAGE_VERTEX_BIT;
				case SHADERSTAGE_PIXEL:
					return VK_SHADER_STAGE_FRAGMENT_BIT;
				case SHADERSTAGE_COMPUTE:
					return VK_SHADER_STAGE_COMPUTE_BIT;
				}
				throw love::Exception("invalid type");
			}

			static EShLanguage getGlslShaderType(ShaderStageType stage) {
				switch (stage) {
				case SHADERSTAGE_VERTEX:
					return EShLangVertex;
				case SHADERSTAGE_PIXEL:
					return EShLangFragment;
				case SHADERSTAGE_COMPUTE:
					return EShLangCompute;
				default:
					throw love::Exception("unkonwn shader stage type");
				}
			}

			Shader::Shader(StrongRef<love::graphics::ShaderStage> stages[])
				: graphics::Shader(stages) {
				loadVolatile();
			}

			bool Shader::loadVolatile() {
				using namespace glslang;
				using namespace spirv_cross;

				TProgram* program = new TProgram();

				auto gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);

				for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++) {
					if (!stages[i])
						continue;

					auto stage = (ShaderStageType)i;
					auto glslangShaderStage = getGlslShaderType(stage);
					auto tshader = new TShader(glslangShaderStage);

					tshader->setEnvInput(EShSourceGlsl, glslangShaderStage, EShClientVulkan, 450);
					tshader->setEnvClient(EShClientVulkan, EShTargetVulkan_1_2);
					tshader->setEnvTarget(EshTargetSpv, EShTargetSpv_1_5);
					tshader->setAutoMapLocations(true);
					tshader->setAutoMapBindings(true);
					tshader->setEnvInputVulkanRulesRelaxed();
					tshader->setGlobalUniformBinding(0);
					tshader->setGlobalUniformSet(0);

					auto& glsl = stages[i]->getSource();
					const char* csrc = glsl.c_str();
					const int sourceLength = glsl.length();
					tshader->setStringsWithLengths(&csrc, &sourceLength, 1);

					int defaultVersio = 450;
					EProfile defaultProfile = ECoreProfile;
					bool forceDefault = false;
					bool forwardCompat = true;

					if (!tshader->parse(&defaultTBuiltInResource, defaultVersio, defaultProfile, forceDefault, forwardCompat, EShMsgSuppressWarnings)) {
						const char* msg1 = tshader->getInfoLog();
						const char* msg2 = tshader->getInfoDebugLog();

						throw love::Exception("error while parsing shader");
					}

					program->addShader(tshader);
				}

				if (!program->link(EShMsgDefault)) {
					throw love::Exception("link failed! %s\n", program->getInfoLog());
				}

				if (!program->mapIO()) {
					throw love::Exception("mapIO failed");
				}

				for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++) {
					auto glslangStage = getGlslShaderType((ShaderStageType)i);
					auto intermediate = program->getIntermediate(glslangStage);
					if (intermediate == nullptr) {
						continue;
					}

					spv::SpvBuildLogger logger;
					glslang::SpvOptions opt;
					opt.validate = true;

					std::vector<uint32_t> spirv;
					GlslangToSpv(*intermediate, spirv, &logger, &opt);

					std::string msgs = logger.getAllMessages();

					VkShaderModuleCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					createInfo.codeSize = spirv.size() * sizeof(uint32_t);
					createInfo.pCode = spirv.data();

					Graphics* vkGfx = (Graphics*)gfx;
					auto device = vkGfx->getDevice();

					VkShaderModule shaderModule;

					if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
						throw love::Exception("failed to create shader module");
					}

					shaderModules.push_back(shaderModule);

					VkPipelineShaderStageCreateInfo shaderStageInfo{};
					shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
					shaderStageInfo.stage = getStageBit((ShaderStageType)i);
					shaderStageInfo.module = shaderModule;
					shaderStageInfo.pName = "main";

					shaderStages.push_back(shaderStageInfo);
				}

				return true;
			}

			void Shader::unloadVolatile() {
				if (shaderModules.size() == 0) {
					return;
				}

				auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
				auto device = gfx->getDevice();
				// fixme: we shouldn't do a greedy wait here.
				vkDeviceWaitIdle(device);
				for (const auto shaderModule : shaderModules) {
					vkDestroyShaderModule(device, shaderModule, nullptr);
				}
				shaderModules.clear();
				shaderStages.clear();
			}

			Shader::~Shader() {
				unloadVolatile();
			}

			void Shader::attach() {
				if (Shader::current != this) {
					Graphics::flushBatchedDrawsGlobal();
					Shader::current = this;
					Vulkan::shaderSwitch();
				}
			}

			int Shader::getVertexAttributeIndex(const std::string& name) {
				return vertexAttributeIndices.at(name);
			}
		}
	}
}
