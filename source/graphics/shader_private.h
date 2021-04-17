#pragma once

#include "../foundation/typeinfo.h"
#include "shader.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DescriptorPoolPrivate : DescriptorPool
		{
			DevicePrivate* device;
			VkDescriptorPool vk_descriptor_pool;

			DescriptorPoolPrivate(DevicePrivate* device);
			~DescriptorPoolPrivate();

			void release() override { delete this; }
		};

		struct DescriptorBinding
		{
			DescriptorType type = DescriptorMax;
			uint count;
			std::string name;

			UdtInfo* ti = nullptr;
		};

		struct DescriptorSetLayoutPrivate : DescriptorSetLayout
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			UniPtr<TypeInfoDataBase> tidb;
			std::vector<DescriptorBinding> bindings;

			VkDescriptorSetLayout vk_descriptor_set_layout;

			DescriptorSetLayoutPrivate(DevicePrivate* device, std::span<const DescriptorBindingInfo> bindings);
			DescriptorSetLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::vector<DescriptorBinding>& bindings, TypeInfoDataBase* db);
			~DescriptorSetLayoutPrivate();

			void release() override { delete this; }

			uint get_bindings_count() const override { return bindings.size(); }
			void get_binding(uint binding, DescriptorBindingInfo* ret) const override;
			int find_binding(const std::string& name) const;
			int find_binding(const char* name) const override { return find_binding(std::string(name)); }

			static DescriptorSetLayoutPrivate* get(DevicePrivate* device, const std::filesystem::path& filename);
		};

		struct DescriptorSetPrivate : DescriptorSet
		{
			DescriptorPoolPrivate* pool;
			DescriptorSetLayoutPrivate* layout;
			VkDescriptorSet vk_descriptor_set;

			DescriptorSetPrivate(DescriptorPoolPrivate* pool, DescriptorSetLayoutPrivate* layout);
			~DescriptorSetPrivate();

			void release() override { delete this; }

			DescriptorSetLayoutPtr get_layout() const override { return layout; }

			void set_buffer(uint binding, uint index, BufferPtr buf, uint offset = 0, uint range = 0) override;
			void set_image(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp) override;
		};

		struct PipelineLayoutPrivate : PipelineLayout
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			std::vector<std::pair<std::string, DescriptorSetLayoutPrivate*>> descriptor_set_layouts;

			UniPtr<TypeInfoDataBase> tidb;
			UdtInfo* pcti = nullptr;
			uint push_constant_size = 0;

			VkPipelineLayout vk_pipeline_layout;

			PipelineLayoutPrivate(DevicePrivate* device, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, uint push_constant_size);
			PipelineLayoutPrivate(DevicePrivate* device, const std::filesystem::path& filename, std::span<DescriptorSetLayoutPrivate*> descriptor_set_layouts, TypeInfoDataBase* db, UdtInfo* pcti);
			~PipelineLayoutPrivate();

			void release() override { delete this; }

			static PipelineLayoutPrivate* get(DevicePrivate* device, const std::filesystem::path& filename);
		};

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
