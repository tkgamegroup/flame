#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/shader.h>
#include "graphics_private.h"

namespace flame
{
	struct UdtInfo;
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

		struct DescriptorsetlayoutPrivate : Descriptorsetlayout
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout v;
#elif defined(FLAME_D3D12)

#endif
			DescriptorsetlayoutPrivate(Device* d, const std::vector<void*>& _bindings);
			~DescriptorsetlayoutPrivate();
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate* p;
#if defined(FLAME_VULKAN)
			VkDescriptorSet v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorsetPrivate(Descriptorpool* p, Descriptorsetlayout* l);
			~DescriptorsetPrivate();

			void set_uniformbuffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			void set_storagebuffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			void set_imageview(uint binding, uint index, Imageview* iv, Sampler* sampler);
			void set_storageimage(uint binding, uint index, Imageview* iv);
		};

		struct PipelinelayoutPrivate : Pipelinelayout
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkPipelineLayout v;
#elif defined(FLAME_D3D12)

#endif

			std::vector<DescriptorsetPrivate*> dsls;
			uint pc_size;
			UdtInfo* pc_udt;

			PipelinelayoutPrivate(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size, uint push_constant_udt_name_hash);
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
				uint set;
				uint binding;
				uint count;
				std::string name;

				Variable v;

				Resource() :
					location(0),
					set(0),
					binding(0),
					count(0)
				{
				}
			};

			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkShaderModule v;
#elif defined(FLAME_D3D12)

#endif
			ShaderStage$ stage;

			std::vector<std::unique_ptr<Resource>> vias;
			std::unique_ptr<Resource> pc;

			ShaderPrivate(Device* d, const std::wstring& filename, const std::string& prefix, Pipelinelayout* pll, bool autogen_code);
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

			PipelinePrivate(Device* d, const GraphicsPipelineInfo& info);
			PipelinePrivate(Device* d, const ComputePipelineInfo& info);
			~PipelinePrivate();
		};
	}
}
