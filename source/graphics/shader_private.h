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
#if USE_D3D12
			ID3D12DescriptorHeap* d3d12_srv_heap = nullptr;
			SparseRanges d3d12_srv_free_list;
			ID3D12DescriptorHeap* d3d12_sp_heap = nullptr;
			SparseRanges d3d12_sp_free_list;
#elif USE_VULKAN
			VkDescriptorPool vk_descriptor_pool = 0;
#endif

			~DescriptorPoolPrivate();
		};

		struct DescriptorSetLayoutPrivate : DescriptorSetLayout
		{
#if USE_VULKAN
			VkDescriptorSetLayout vk_descriptor_set_layout = 0;
#endif
			TypeInfoDataBase db;

			~DescriptorSetLayoutPrivate();

			static DescriptorSetLayoutPtr load_from_res(const std::filesystem::path& filename);
		};

		struct DescriptorSetPrivate : DescriptorSet
		{
			struct BufRes
			{
				BufferPtr buf;
				uint offset;
				uint range;
			};

			struct ImgRes
			{
				ImageViewPtr iv;
				SamplerPtr sp;
			};

			union Res
			{
				BufRes b;
				ImgRes i;
			};

#if USE_D3D12
			int d3d12_rtv_off = -1;
			uint d3d12_rtv_num = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_srv_cpu_handle;
			D3D12_GPU_DESCRIPTOR_HANDLE d3d12_srv_gpu_handle;
			int d3d12_sp_off = -1;
			uint d3d12_sp_num = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE d3d12_sp_cpu_handle;
			D3D12_GPU_DESCRIPTOR_HANDLE d3d12_sp_gpu_handle;
#elif USE_VULKAN
			VkDescriptorSet vk_descriptor_set = 0;
#endif
			DescriptorPoolPtr pool;

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
#if USE_D3D12
			ID3D12RootSignature* d3d12_root_signature = nullptr;
#elif USE_VULKAN
			VkPipelineLayout vk_pipeline_layout = 0;
#endif
			TypeInfoDataBase db;

			~PipelineLayoutPrivate();

			static PipelineLayoutPtr load_from_res(const std::filesystem::path& filename);
		};

		struct ShaderPrivate : Shader
		{
#if USE_D3D12
			std::string dxbc;
#elif USE_VULKAN
			VkShaderModule vk_module = 0;
#endif
			TypeInfoDataBase db;

			ShaderPrivate();
			~ShaderPrivate();
			void recreate() override;

			static ShaderPtr load_from_res(const std::filesystem::path& filename);
		};

		void load_pipeline(PipelineType pipeline_type, const std::filesystem::path& filename, const std::vector<std::string>& defines, void** ret);

		struct GraphicsPipelinePrivate : GraphicsPipeline
		{
#if USE_D3D12
			ID3D12PipelineState* d3d12_pipeline = nullptr;
#elif USE_VULKAN
			VkPipeline vk_pipeline = 0;
			std::unordered_map<RenderpassPtr, VkPipeline> renderpass_variants;
#endif

			GraphicsPipelinePrivate();
			~GraphicsPipelinePrivate();
#if USE_VULKAN
			VkPipeline get_dynamic_pipeline(RenderpassPtr rp, uint sp);
#endif
			void recreate() override;
		};

		struct ComputePipelinePrivate : ComputePipeline
		{
#if USE_D3D12
			ID3D12PipelineState* d3d12_pipeline = nullptr;
#elif USE_VULKAN
			VkPipeline vk_pipeline = 0;
#endif

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
