#pragma once

#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>
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

		struct DescriptorBinding
		{
			DescriptorType type = DescriptorMax;
			uint count;
			std::string name;

			UdtInfo* ti = nullptr;
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

			FlmPtr<TypeInfoDataBase> tidb;
			std::vector<DescriptorBinding> bindings;

			VkDescriptorSetLayout vk_descriptor_set_layout;

			DescriptorSetLayoutPrivate(DevicePrivate* device, std::span<const DescriptorBindingInfo> bindings);
			DescriptorSetLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::vector<DescriptorBinding>& bindings, TypeInfoDataBase* db);
			~DescriptorSetLayoutPrivate();

			void release() override { delete this; }

			uint get_bindings_count() const override { return bindings.size(); }
			void get_binding(uint binding, DescriptorBindingInfo* ret) const override;
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

		struct ShaderPrivate : Shader
		{
			std::filesystem::path filename;
			std::vector<std::string> defines;
			std::vector<std::pair<std::string, std::string>> substitutes;
			ShaderStageFlags type = ShaderStageNone;

			DevicePrivate* device;

			VkShaderModule vk_module = 0;

			ShaderPrivate(DevicePrivate* device, const std::filesystem::path& filename, const std::vector<std::string>& defines, const std::vector<std::pair<std::string, std::string>>& substitutes, const std::string& spv_content);
			~ShaderPrivate();

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return filename.c_str(); }

			static ShaderPrivate* get(DevicePrivate* device, const std::filesystem::path& filename, const std::string& defines = "", const std::string& substitutes = "");
			static ShaderPrivate* get(DevicePrivate* device, const std::filesystem::path& filename, const std::vector<std::string>& defines, const std::vector<std::pair<std::string, std::string>>& substitutes);
		};

		struct PipelineLayoutPrivate : PipelineLayout
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			std::vector<DescriptorSetLayoutPrivate*> descriptor_set_layouts;
			std::unordered_map<uint64, uint> descriptor_set_layouts_map;

			FlmPtr<TypeInfoDataBase> tidb;
			UdtInfo* pcti = nullptr;
			uint push_constant_size = 0;

			VkPipelineLayout vk_pipeline_layout;

			PipelineLayoutPrivate(DevicePrivate* device, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, uint push_constant_size);
			PipelineLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, TypeInfoDataBase* db, UdtInfo* pcti);
			~PipelineLayoutPrivate();

			void release() override { delete this; }

			uint get_idx(uint64 h) { return descriptor_set_layouts_map[h]; };

			static PipelineLayoutPrivate* get(DevicePrivate* device, const std::filesystem::path& filename);
		};

		struct PipelinePrivate : Pipeline
		{
			PipelineType type;

			std::filesystem::path filename;

			DevicePrivate* device;
			PipelineLayoutPrivate* pipeline_layout;
			std::vector<ShaderPrivate*> shaders;

			VkPipeline vk_pipeline;

			PipelinePrivate(DevicePrivate* device, std::span<ShaderPrivate*> shaders, PipelineLayoutPrivate* pll, const GraphicsPipelineInfo& info);
			PipelinePrivate(DevicePrivate* device, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll);
			~PipelinePrivate();

			static PipelinePrivate* create(DevicePrivate* device, std::span<ShaderPrivate*> shaders, PipelineLayoutPrivate* pll, const GraphicsPipelineInfo& info);
			static PipelinePrivate* create(DevicePrivate* device, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll);
			static PipelinePrivate* get(DevicePrivate* device, const std::filesystem::path& filename);

			void release() override { delete this; }

			PipelineType get_type() const override { return type; }
		};
	}
}
