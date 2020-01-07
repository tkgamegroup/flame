#pragma once

#include <flame/serialize.h>
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

			std::vector<DescriptorBindingBase*> bindings;
			Descriptorset* default_set;

			DescriptorlayoutPrivate(Device* d, uint binding_count, DescriptorBindingBase* const* bindings, Descriptorpool* default_set_pool);
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

			PipelinelayoutPrivate(Device* d, uint descriptorlayout_count, Descriptorlayout* const* descriptorlayouts, uint push_constant_size);
			~PipelinelayoutPrivate();
		};

		struct StageInfo
		{
			struct InOut
			{
				std::string name;
				std::string formated_type;

				bool blend_enable;
				BlendFactor$ blend_src_color;
				BlendFactor$ blend_dst_color;
				BlendFactor$ blend_src_alpha;
				BlendFactor$ blend_dst_alpha;
				/*
					if (blendEnable) 
					{
						finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
						finalColor.a   = (srcAlphaBlendFactor * newColor.a  ) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a  );
					}
					else
						finalColor = newColor;
 
					finalColor = finalColor & colorWriteMask;
				*/

				InOut(const std::string& name, const std::string& type) :
					name(name),
					formated_type(type),
					blend_enable(false),
					blend_src_color(BlendFactorZero),
					blend_dst_color(BlendFactorZero),
					blend_src_alpha(BlendFactorZero),
					blend_dst_alpha(BlendFactorZero)
				{
					if (type[0] == 'i' || type[0] == 'u')
						formated_type = "flat " + formated_type;
				}
			};

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

			std::wstring filename;
			std::string prefix;
			std::filesystem::path path;
			ShaderStage$ type;

			std::vector<InOut> inputs;
			std::vector<InOut> outputs;
			std::vector<std::unique_ptr<Resource>> uniform_buffers;
			std::unique_ptr<Resource> push_constant;

#if defined(FLAME_VULKAN)
			VkShaderModule vk_shader_module;
#elif defined(FLAME_D3D12)

#endif

			StageInfo(const std::wstring& fn)
			{
				auto sp = ssplit(fn, L':');
				filename = sp[0];
				if (sp.size() > 1)
					prefix = w2s(sp[1]);
				path = std::filesystem::canonical(filename);
				type = shader_stage_from_ext(path.extension());
				vk_shader_module = 0;
			}
		};

		struct PipelinePrivate : Pipeline
		{
			DevicePrivate* d;
			PipelinelayoutPrivate* pll;

#if defined(FLAME_VULKAN)
			std::vector<VkShaderModule> vk_shader_modules;
			VkPipeline v;
#elif defined(FLAME_D3D12)

#endif

			PipelinePrivate(DevicePrivate* d, const std::vector<StageInfo>& stage_infos, PipelinelayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInputInfo* vi, const Vec2u& vp, RasterInfo* raster, SampleCount$ sc, DepthInfo* depth, uint dynamic_state_count, const uint* dynamic_states);
			PipelinePrivate(DevicePrivate* d, const StageInfo& compute_stage_info, PipelinelayoutPrivate* pll);
			~PipelinePrivate();
		};
	}
}
