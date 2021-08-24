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
			DescriptorType type = Descriptor_Max;
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
			struct BufRes
			{
				BufferPrivate* p;
				uint offset;
				uint range;
			};

			struct ImgRes
			{
				ImageViewPrivate* p;
				SamplerPrivate* sp;
			};

			union Res
			{
				BufRes b;
				ImgRes i;
			};

			DevicePrivate* device;
			DescriptorPoolPrivate* pool;
			DescriptorSetLayoutPrivate* layout;
			VkDescriptorSet vk_descriptor_set;

			std::vector<std::vector<Res>> reses;
			std::vector<std::pair<uint, uint>> buf_updates;
			std::vector<std::pair<uint, uint>> img_updates;

			DescriptorSetPrivate(DescriptorPoolPrivate* pool, DescriptorSetLayoutPrivate* layout);
			~DescriptorSetPrivate();

			void release() override { delete this; }

			DescriptorSetLayoutPtr get_layout() const override { return layout; }

			void set_buffer(uint binding, uint index, BufferPtr buf, uint offset = 0, uint range = 0) override;
			void set_image(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp) override;
			void update() override;
		};

		struct PipelineLayoutPrivate : PipelineLayout
		{
			DevicePrivate* device;

			std::filesystem::path filename;

			std::vector<std::pair<std::string, DescriptorSetLayoutPrivate*>> descriptor_set_layouts;

			UniPtr<TypeInfoDataBase> tidb;
			UdtInfo* pcti = nullptr;
			uint pc_sz = 0;

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
			PipelineLayoutPrivate* layout;
			RenderpassPrivate* rp = nullptr;
			std::vector<ShaderPrivate*> shaders;

			VkPipeline vk_pipeline;

			PipelinePrivate(DevicePrivate* device, const GraphicsPipelineInfo& info);
			PipelinePrivate(DevicePrivate* device, const ComputePipelineInfo& info);
			~PipelinePrivate();

			static PipelinePrivate* create(DevicePrivate* device, const GraphicsPipelineInfo& info);
			static PipelinePrivate* create(DevicePrivate* device, const ComputePipelineInfo& info);
			static PipelinePrivate* get(DevicePrivate* device, const std::filesystem::path& filename);

			void release() override { delete this; }

			PipelineType get_type() const override { return type; }
		};
	}
}
