#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/shader.h>
#include "graphics_private.h"

namespace flame
{
	struct SerializableNode;

	namespace graphics
	{
		struct DevicePrivate;
		struct RenderpassPrivate;

		struct DescriptorpoolPrivate : Descriptorpool
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkDescriptorPool v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorpoolPrivate(Device* d);
			~DescriptorpoolPrivate();
		};

		struct DescriptorlayoutPrivate : Descriptorlayout
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout v;
#elif defined(FLAME_D3D12)

#endif

			std::vector<DescriptorBinding> bindings_map;

			DescriptorlayoutPrivate(Device* d, const std::vector<void*>& _bindings);
			~DescriptorlayoutPrivate();
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate* p;
			DescriptorlayoutPrivate* l;
#if defined(FLAME_VULKAN)
			VkDescriptorSet v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorsetPrivate(Descriptorpool* p, Descriptorlayout* l);
			~DescriptorsetPrivate();

			void set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range);
			void set_image(uint binding, uint index, Imageview* iv, Sampler* sampler);
		};

		struct PipelinelayoutPrivate : Pipelinelayout
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkPipelineLayout v;
#elif defined(FLAME_D3D12)

#endif

			std::vector<DescriptorlayoutPrivate*> dsls;
			uint pc_size;
			UdtInfo* pc_udt;

			PipelinelayoutPrivate(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, UdtInfo* push_constant_udt);
			~PipelinelayoutPrivate();
		};

		struct ShaderPrivate : Shader
		{
			struct Variable
			{
				std::string type_name;
				std::string name;
				uint offset;
				uint size;
				uint count;
				uint array_stride;

				std::vector<std::unique_ptr<Variable>> members;

				Variable() :
					offset(0),
					size(0),
					count(0),
					array_stride(0)
				{
				}
			};

			struct Resource
			{
				uint location;
				uint index;
				uint set;
				uint binding;
				std::string name;

				Variable v;

				Resource() :
					location(0),
					index(0),
					set(0),
					binding(0)
				{
				}
			};

			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkShaderModule v;
#elif defined(FLAME_D3D12)

#endif
			ShaderStage$ stage;

			std::vector<std::unique_ptr<Resource>> inputs;
			std::vector<std::unique_ptr<Resource>> outputs;
			std::vector<std::unique_ptr<Resource>> uniform_buffers;
			std::vector<std::unique_ptr<Resource>> storage_buffers;
			std::vector<std::unique_ptr<Resource>> sampled_images;
			std::vector<std::unique_ptr<Resource>> storage_images;
			std::unique_ptr<Resource> push_constant;

			ShaderPrivate(Device* d, const std::wstring& filename, const std::string& prefix, const std::vector<void*>& inputs, const std::vector<void*>& outputs, Pipelinelayout* pll, bool autogen_code);
			~ShaderPrivate();
		};

		struct PipelinePrivate : Pipeline
		{
			DevicePrivate* d;
			PipelinelayoutPrivate* pll;

#if defined(FLAME_VULKAN)
			VkPipeline v;
#elif defined(FLAME_D3D12)

#endif

			PipelinePrivate(Device* d, const std::vector<void*>& shaders, Pipelinelayout* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, const std::vector<void*>& output_states, const std::vector<uint>& dynamic_states);
			PipelinePrivate(Device* d, Shader* compute_shader, Pipelinelayout* pll);
			~PipelinePrivate();
		};
	}
}
