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

// LOVE
#include "common/Object.h"
#include "common/StringMap.h"
#include "Texture.h"
#include "ShaderStage.h"
#include "Resource.h"
#include "Buffer.h"

// STL
#include <string>
#include <map>
#include <vector>
#include <stddef.h>

namespace love
{
namespace graphics
{

class Graphics;
class Buffer;

// A GLSL shader
class Shader : public Object, public Resource
{
public:

	static love::Type type;

	enum Language
	{
		LANGUAGE_GLSL3,
		LANGUAGE_GLSL4,
		LANGUAGE_MAX_ENUM
	};

	// Built-in uniform variables.
	enum BuiltinUniform
	{
		BUILTIN_TEXTURE_MAIN,
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
		BUILTIN_UNIFORMS_PER_DRAW,
		BUILTIN_MAX_ENUM
	};

	// Types of potential uniform variables used in love's shaders.
	enum UniformType
	{
		UNIFORM_FLOAT,
		UNIFORM_MATRIX,
		UNIFORM_INT,
		UNIFORM_UINT,
		UNIFORM_BOOL,
		UNIFORM_SAMPLER,
		UNIFORM_STORAGETEXTURE,
		UNIFORM_TEXELBUFFER,
		UNIFORM_STORAGEBUFFER,
		UNIFORM_UNKNOWN,
		UNIFORM_MAX_ENUM
	};

	enum StandardShader
	{
		STANDARD_DEFAULT,
		STANDARD_VIDEO,
		STANDARD_ARRAY,
		STANDARD_POINTS,
		STANDARD_MAX_ENUM
	};

	enum EntryPoint
	{
		ENTRYPOINT_NONE,
		ENTRYPOINT_HIGHLEVEL,
		ENTRYPOINT_CUSTOM,
		ENTRYPOINT_RAW,
	};

	enum Access
	{
		ACCESS_NONE = 0,
		ACCESS_READ = (1 << 0),
		ACCESS_WRITE = (1 << 1),
	};

	enum ClipSpaceTransformFlags
	{
		CLIP_TRANSFORM_NONE = 0,
		CLIP_TRANSFORM_FLIP_Y = 1 << 0,
		CLIP_TRANSFORM_Z_NEG1_1_TO_0_1 = 1 << 1,
		CLIP_TRANSFORM_Z_0_1_TO_NEG1_1 = 1 << 2,
	};

	struct CompileOptions
	{
		std::map<std::string, std::string> defines;
		std::string debugName;
	};

	struct SourceInfo
	{
		Language language;
		EntryPoint stages[SHADERSTAGE_MAX_ENUM];
		bool usesMRT;
	};

	struct MatrixSize
	{
		short columns;
		short rows;
	};

	struct UniformInfo
	{
		UniformType baseType;
		uint32 stageMask;
		bool active;

		int location;
		int count;

		union
		{
			int components;
			MatrixSize matrix;
		};

		DataBaseType dataBaseType;
		TextureType textureType;
		Access access;
		bool isDepthSampler;
		PixelFormat storageTextureFormat;
		size_t bufferStride;
		size_t bufferMemberCount;
		std::string name;

		int resourceIndex;
		int bindingStartIndex;

		union
		{
			void *data;
			float *floats;
			int *ints;
			unsigned int *uints;
		};

		size_t dataSize;
	};

	union LocalUniformValue
	{
		float f;
		int32 i;
		uint32 u;
	};

	// The members in here must respect uniform buffer alignment/padding rules.
 	struct BuiltinUniformData
 	{
 		Matrix4 transformMatrix;
 		Matrix4 projectionMatrix;
		Vector4 scaleParams;
		Vector4 clipSpaceParams;
 		Colorf constantColor;
		Vector4 screenSizeParams;
 	};

	// Pointer to currently active Shader.
	static Shader *current;

	// Pointer to the default Shader.
	static Shader *standardShaders[STANDARD_MAX_ENUM];

	Shader(StrongRef<ShaderStage> stages[], const CompileOptions &options);
	virtual ~Shader();

	/**
	 * Check whether a Shader has a stage.
	 **/
	bool hasStage(ShaderStageType stage);

	/**
	 * Binds this Shader's program to be used when rendering.
	 **/
	virtual void attach() = 0;

	/**
	 * Attach a default shader.
	 **/
	static void attachDefault(StandardShader defaultType);

	/**
	 * Gets whether any of the default shaders are currently active.
	 **/
	static bool isDefaultActive();

	/**
	 * Used for transforming standardized post-projection clip space positions
	 * into the backend's current clip space.
	 * Right now, the standard is:
	 *   NDC y is [-1, 1] starting at the bottom (y-up).
	 *   NDC z is [-1, 1].
	 *   Pixel coordinates are y-down.
	 *   Pixel (0, 0) in a texture is the top-left.
	 * Aside from NDC z, this matches Metal and D3D12.
	 */
	static Vector4 computeClipSpaceParams(uint32 clipSpaceTransformFlags);

	/**
	 * Returns any warnings this Shader may have generated.
	 **/
	virtual std::string getWarnings() const = 0;

	const std::string &getDebugName() const { return debugName; }

	virtual int getVertexAttributeIndex(const std::string &name) = 0;

	const UniformInfo *getUniformInfo(const std::string &name) const;
	virtual const UniformInfo *getUniformInfo(BuiltinUniform builtin) const = 0;

	virtual void updateUniform(const UniformInfo *info, int count) = 0;

	void sendTextures(const UniformInfo *info, Texture **textures, int count);
	void sendBuffers(const UniformInfo *info, Buffer **buffers, int count);

	/**
	 * Gets whether a uniform with the specified name exists and is actively
	 * used in the shader.
	 **/
	bool hasUniform(const std::string &name) const;

	/**
	 * Sets the textures used when rendering a video. For internal use only.
	 **/
	void setVideoTextures(Texture *ytexture, Texture *cbtexture, Texture *crtexture);

	const UniformInfo *getMainTextureInfo() const;
	void validateDrawState(PrimitiveType primtype, Texture *maintexture) const;

	void getLocalThreadgroupSize(int *x, int *y, int *z);

	const std::vector<Buffer::DataDeclaration> *getBufferFormat(const std::string &name) const;

	bool isUsingDeprecatedTextureFunctions() const;
	bool isUsingDeprecatedTextureUniform() const;

	const std::string& getUnsetVertexInputLocationsString() const { return unsetVertexInputLocationsString; }

	static SourceInfo getSourceInfo(const std::string &src);
	static std::string createShaderStageCode(Graphics *gfx, ShaderStageType stage, const std::string &code, const CompileOptions &options, const SourceInfo &info, bool gles, bool checksystemfeatures);

	static bool validate(StrongRef<ShaderStage> stages[], std::string &err);

	static bool initialize();
	static void deinitialize();

	static const std::string &getDefaultCode(StandardShader shader, ShaderStageType stage);

	static bool getConstant(const char *in, Language &out);
	static bool getConstant(Language in, const char *&out);

	static bool getConstant(const char *in, BuiltinUniform &out);
	static bool getConstant(BuiltinUniform in, const char *&out);

protected:

	struct Reflection
	{
		std::map<std::string, int> vertexInputs;

		std::map<std::string, UniformInfo> texelBuffers;
		std::map<std::string, UniformInfo> storageBuffers;
		std::map<std::string, UniformInfo> sampledTextures;
		std::map<std::string, UniformInfo> storageTextures;
		std::map<std::string, UniformInfo> localUniforms;

		std::map<std::string, UniformInfo *> allUniforms;

		std::map<std::string, std::vector<LocalUniformValue>> localUniformInitializerValues;

		std::map<std::string, std::vector<Buffer::DataDeclaration>> bufferFormats;

		int textureCount;
		int bufferCount;

		int localThreadgroupSize[3];
		bool usesPointSize;
	};

	std::string getShaderStageDebugName(ShaderStageType stage) const;

	void handleUnknownUniformName(const char *name);

	// std140 uniform buffer alignment-aware copy.
	void copyToUniformBuffer(const UniformInfo *info, const void *src, void *dst, int count) const;

	void sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalupdate);
	void sendBuffers(const UniformInfo *info, Buffer **buffers, int count, bool internalupdate);

	virtual void applyTexture(const UniformInfo *info, int i, Texture *texture, UniformType basetype, bool isdefault) = 0;
	virtual void applyBuffer(const UniformInfo *info, int i, Buffer *buffer, UniformType basetype, bool isdefault) = 0;

	void flushBatchedDraws() const;

	static std::string canonicaliizeUniformName(const std::string &name);
	static bool validateInternal(StrongRef<ShaderStage> stages[], std::string& err, Reflection &reflection);
	static DataBaseType getDataBaseType(PixelFormat format);
	static bool isResourceBaseTypeCompatible(DataBaseType a, DataBaseType b);

	static bool validateTexture(const UniformInfo *info, Texture *tex, bool internalUpdate);
	static bool validateBuffer(const UniformInfo *info, Buffer *buffer, bool internalUpdate);

	StrongRef<ShaderStage> stages[SHADERSTAGE_MAX_ENUM];

	Reflection reflection;

	std::vector<Texture *> activeTextures;
	std::vector<Buffer *> activeBuffers;

	std::string debugName;

	std::string unsetVertexInputLocationsString;

}; // Shader

} // graphics
} // love
