#pragma once

#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/graphics/shader.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct RenderpassPrivate;
		struct DescriptorSetPrivate;

		struct ShaderType;

		struct ShaderVariable
		{
			std::string name;
			uint offset = 0;
			uint size = 0;
			uint array_count = 0;
			uint array_stride = 0;

			ShaderType* info = nullptr;
		};

		enum ShaderBaseType
		{
			ShaderBaseTypeInt,
			ShaderBaseTypeVec2i,
			ShaderBaseTypeVec3i,
			ShaderBaseTypeVec4i,
			ShaderBaseTypeUint,
			ShaderBaseTypeVec2u,
			ShaderBaseTypeVec3u,
			ShaderBaseTypeVec4u,
			ShaderBaseTypeFloat,
			ShaderBaseTypeVec2f,
			ShaderBaseTypeVec3f,
			ShaderBaseTypeVec4f,
			ShaderBaseTypeMat2f,
			ShaderBaseTypeMat3f,
			ShaderBaseTypeMat4f,
			ShaderBaseTypeStruct,
			ShaderBaseTypeImage
		};

		struct ShaderType
		{
			uint id;
			ShaderBaseType base_type;
			std::string name;
			std::vector<ShaderVariable> variables;
		};

		struct DescriptorBindingPrivate : DescriptorBinding
		{
			DescriptorType type;
			uint binding;
			uint count;
			std::string name;

			ShaderType* info = nullptr;

			DescriptorBindingPrivate() {}
			DescriptorBindingPrivate(uint index, const DescriptorBindingInfo& info);

			uint get_binding() const override { return binding; }
			DescriptorType get_type() const override { return type; }
			uint get_count() const override { return count; }
			const char* get_name() const override { return name.c_str(); }
		};

		struct ShaderInOutInfo
		{
			uint location;
			std::string name;
			std::string type;
		};

		struct DescriptorPoolPrivate : DescriptorPool
		{
			DevicePrivate* device;
			VkDescriptorPool vk_descriptor_pool;

			DescriptorPoolPrivate(DevicePrivate* device);
			~DescriptorPoolPrivate();

			void release() override { delete this; }
		};

		struct DescriptorSetLayoutPrivate : DescriptorSetLayout
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			std::vector<std::unique_ptr<ShaderType>> types;
			std::vector<std::unique_ptr<DescriptorBindingPrivate>> bindings;

			VkDescriptorSetLayout vk_descriptor_set_layout;

			DescriptorSetLayoutPrivate(DevicePrivate* device, std::span<const DescriptorBindingInfo> bindings);
			DescriptorSetLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::vector<std::unique_ptr<ShaderType>>& types, std::vector<std::unique_ptr<DescriptorBindingPrivate>>& bindings);
			~DescriptorSetLayoutPrivate();

			void release() override { delete this; }

			uint get_bindings_count() const override { return bindings.size(); }
			DescriptorBinding* get_binding(uint binding) const override { return bindings[binding].get(); }

			static DescriptorSetLayoutPrivate* get(const std::filesystem::path& filename);
			static DescriptorSetLayoutPrivate* create(DevicePrivate* device, const std::filesystem::path& filename);
		};

		extern std::vector<std::unique_ptr<DescriptorSetLayoutPrivate>> descriptor_set_layouts;

		struct DescriptorSetBridge : DescriptorSet
		{
			void set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range) override;
			void set_image(uint binding, uint index, ImageView* v, Sampler* sampler) override;
		};

		struct DescriptorSetPrivate : DescriptorSetBridge
		{
			DescriptorPoolPrivate* descriptor_pool;
			DescriptorSetLayoutPrivate* descriptor_layout;
			VkDescriptorSet vk_descriptor_set;

			DescriptorSetPrivate(DescriptorPoolPrivate* p, DescriptorSetLayoutPrivate* l);
			~DescriptorSetPrivate();

			void release() override { delete this; }

			DescriptorSetLayout* get_layout() const override { return descriptor_layout; }

			void set_buffer(uint binding, uint index, BufferPrivate* b, uint offset = 0, uint range = 0);
			void set_image(uint binding, uint index, ImageViewPrivate* iv, SamplerPrivate* sp);
		};

		inline void DescriptorSetBridge::set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range)
		{
			((DescriptorSetPrivate*)this)->set_buffer(binding, index, (BufferPrivate*)b, offset, range);
		}

		inline void DescriptorSetBridge::set_image(uint binding, uint index, ImageView* v, Sampler* sampler)
		{
			((DescriptorSetPrivate*)this)->set_image(binding, index, (ImageViewPrivate*)v, (SamplerPrivate*)sampler);
		}

		struct PipelineLayoutPrivate : PipelineLayout
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			std::vector<DescriptorSetLayoutPrivate*> descriptor_set_layouts;

			std::vector<std::unique_ptr<ShaderType>> types;
			ShaderVariable push_constant;

			VkPipelineLayout vk_pipeline_layout;

			PipelineLayoutPrivate(DevicePrivate* device, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, uint push_constant_size);
			PipelineLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, std::vector<std::unique_ptr<ShaderType>>& types, ShaderVariable& push_constant);
			~PipelineLayoutPrivate();

			void release() override { delete this; }

			static PipelineLayoutPrivate* create(DevicePrivate* device, const std::filesystem::path& filename);
		};

		extern std::vector<std::unique_ptr<PipelineLayoutPrivate>> pipeline_layouts;

		struct ShaderPrivate : Shader
		{
			std::filesystem::path filename;
			std::string defines;
			ShaderStageFlags type;

			DevicePrivate* device;

			//std::vector<ShaderInOutInfo> inputs;
			//std::vector<ShaderInOutInfo> outputs;

			VkShaderModule vk_module = 0;

			ShaderPrivate(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines, const std::string& spv_content);
			~ShaderPrivate();

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return filename.c_str(); }
			const char* get_defines() const override { return defines.c_str(); }

			static ShaderPrivate* create(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines = "");
		};

		struct PipelinePrivate : Pipeline
		{
			PipelineType type;

			DevicePrivate* device;
			PipelineLayoutPrivate* pipeline_layout;
			std::vector<ShaderPrivate*> shaders;

			VkPipeline vk_pipeline;

			PipelinePrivate(DevicePrivate* device, std::span<ShaderPrivate*> shaders, PipelineLayoutPrivate* pll, 
				RenderpassPrivate* rp, uint subpass_idx, VertexInfo* vi = nullptr, RasterInfo* raster = nullptr, DepthInfo* depth = nullptr, 
				std::span<const BlendOption> blend_options = {}, std::span<const uint> dynamic_states = {});
			PipelinePrivate(DevicePrivate* device, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll);
			~PipelinePrivate();

			static PipelinePrivate* create(DevicePrivate* device, std::span<ShaderPrivate*> shaders, PipelineLayoutPrivate* pll, 
				Renderpass* rp, uint subpass_idx, VertexInfo* vi = nullptr, RasterInfo* raster = nullptr, DepthInfo* depth = nullptr, 
				std::span<const BlendOption> blend_options = {}, std::span<const uint> dynamic_states = {});
			static PipelinePrivate* create(DevicePrivate* device, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll);

			void release() override { delete this; }

			PipelineType get_type() const override { return type; }
		};
	}
}
