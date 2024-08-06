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
			VkDescriptorPool vk_descriptor_pool;

			~DescriptorPoolPrivate();
		};

		struct DescriptorSetLayoutPrivate : DescriptorSetLayout
		{
			TypeInfoDataBase db;
			VkDescriptorSetLayout vk_descriptor_set_layout;

			~DescriptorSetLayoutPrivate();

			static DescriptorSetLayoutPtr load_from_res(const std::filesystem::path& filename);
		};

		struct DescriptorSetPrivate : DescriptorSet
		{
			struct BufRes
			{
				void* vk_buf;
				uint offset;
				uint range;
			};

			struct ImgRes
			{
				void* vk_iv;
				void* vk_sp;
			};

			union Res
			{
				BufRes b;
				ImgRes i;
			};

			DescriptorPoolPtr pool;
			VkDescriptorSet vk_descriptor_set;
			ID3D12DescriptorHeap* d3d12_descriptor_heap = nullptr;

			std::vector<std::vector<Res>> reses;
			std::vector<std::pair<uint, uint>> buf_updates;
			std::vector<std::pair<uint, uint>> img_updates;

			~DescriptorSetPrivate();

			void set_buffer_i(uint binding, uint index, BufferPtr buf, uint offset = 0, uint range = 0) override;
			void set_image_i(uint binding, uint index, ImageViewPtr iv, SamplerPtr sp) override;
			void update() override;
		};

		struct PipelineLayoutPrivate : PipelineLayout
		{
			TypeInfoDataBase db;
			VkPipelineLayout vk_pipeline_layout;

			ID3D12RootSignature* d3d12_signature = nullptr;

			~PipelineLayoutPrivate();

			static PipelineLayoutPtr load_from_res(const std::filesystem::path& filename);
		};

		struct ShaderPrivate : Shader
		{
			TypeInfoDataBase db;
			VkShaderModule vk_module = 0;
			std::string d3d12_byte_code;

			ShaderPrivate();
			~ShaderPrivate();
			void recreate() override;

			static ShaderPtr load_from_res(const std::filesystem::path& filename);
		};

		void load_pipeline(PipelineType pipeline_type, const std::filesystem::path& filename, const std::vector<std::string>& defines, void** ret);

		struct GraphicsPipelinePrivate : GraphicsPipeline
		{
			VkPipeline vk_pipeline;
			ID3D12PipelineState* d3d12_pipeline = nullptr;

			std::unordered_map<RenderpassPtr, VkPipeline> renderpass_variants;

			GraphicsPipelinePrivate();
			~GraphicsPipelinePrivate();
			VkPipeline get_dynamic_pipeline(RenderpassPtr rp, uint sp);
			void recreate() override;
		};

		struct ComputePipelinePrivate : ComputePipeline
		{
			VkPipeline vk_pipeline;

			~ComputePipelinePrivate();
		};

		extern std::unique_ptr<DescriptorPoolT>						descriptorset_pool;
		extern std::vector<std::unique_ptr<DescriptorSetLayoutT>>	loaded_descriptorsetlayouts;
		extern std::vector<std::unique_ptr<PipelineLayoutT>>		loaded_pipelinelayouts;
		extern std::vector<ShaderPtr>								shaders;
		extern std::vector<std::unique_ptr<ShaderT>>				loaded_shaders;
		extern std::vector<GraphicsPipelinePtr>						graphics_pipelines;
		extern std::vector<std::unique_ptr<GraphicsPipelineT>>		loaded_graphics_pipelines;
		extern std::vector<std::unique_ptr<ComputePipelineT>>		loaded_compute_pipelines;
	}
}
