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

		enum ShaderTypeTag
		{
			ShaderTagBase,
			ShaderTagStruct,
			ShaderTagImage
		};

		struct ShaderType
		{
			uint id = -1;
			ShaderTypeTag tag = ShaderTagStruct;
			std::string name;
			uint size = 0;
			std::vector<ShaderVariable> variables;
			std::unordered_map<uint64, uint> variables_map;

			void make_map()
			{
				for (auto i = 0; i < variables.size(); i++)
					variables_map[std::hash<std::string>()(variables[i].name)] = i;
			}
		};

		inline ShaderType* find_type(const std::vector<std::unique_ptr<ShaderType>>& types, uint id)
		{
			for (auto& t : types)
			{
				if (t->id == id)
					return t.get();
			}
			return nullptr;
		}

		inline ShaderType* find_type(const std::vector<std::unique_ptr<ShaderType>>& types, const std::string& name)
		{
			for (auto& t : types)
			{
				if (t->name == name)
					return t.get();
			}
			return nullptr;
		}

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

		struct DescriptorSetLayoutBridge : DescriptorSetLayout
		{
			int find_binding(const char* name) const override;
		};

		struct DescriptorSetLayoutPrivate : DescriptorSetLayoutBridge
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
			int find_binding(const std::string& name);

			static DescriptorSetLayoutPrivate* get(DevicePrivate* device, const std::filesystem::path& filename);
		};

		inline int DescriptorSetLayoutBridge::find_binding(const char* name) const
		{
			return ((DescriptorSetLayoutPrivate*)this)->find_binding(name);
		}

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
			std::unordered_map<uint64, uint> descriptor_set_layouts_map;

			std::vector<std::unique_ptr<ShaderType>> types;
			ShaderType* push_constant = nullptr;

			VkPipelineLayout vk_pipeline_layout;

			PipelineLayoutPrivate(DevicePrivate* device, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, uint push_constant_size);
			PipelineLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, std::vector<std::unique_ptr<ShaderType>>& types, ShaderType* push_constant);
			~PipelineLayoutPrivate();

			void release() override { delete this; }

			uint get_idx(uint64 h) { return descriptor_set_layouts_map[h]; };

			static PipelineLayoutPrivate* get(DevicePrivate* device, const std::filesystem::path& filename);
		};

		struct ShaderPrivate : Shader
		{
			std::filesystem::path filename;
			std::vector<std::string> defines;
			std::vector<std::pair<std::string, std::string>> substitutes;
			ShaderStageFlags type = ShaderStageNone;

			DevicePrivate* device;

			//std::vector<ShaderInOutInfo> inputs;
			//std::vector<ShaderInOutInfo> outputs;

			VkShaderModule vk_module = 0;

			ShaderPrivate(DevicePrivate* device, const std::filesystem::path& filename, const std::vector<std::string>& defines, const std::vector<std::pair<std::string, std::string>>& substitutes, const std::string& spv_content);
			~ShaderPrivate();

			void release() override;

			const wchar_t* get_filename() const override { return filename.c_str(); }

			static std::vector<std::string> format_defines(const std::string& defines);
			static std::vector<std::pair<std::string, std::string>> format_substitutes(const std::string& substitutes);
			static ShaderPrivate* get(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines = "", const std::string& substitutes = "", const std::vector<std::filesystem::path>& extra_dependencies = {});
			static ShaderPrivate* get(DevicePrivate* device, const std::filesystem::path& filename, const std::vector<std::string>& defines, const std::vector<std::pair<std::string, std::string>>& substitutes, const std::vector<std::filesystem::path>& extra_dependencies);
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
