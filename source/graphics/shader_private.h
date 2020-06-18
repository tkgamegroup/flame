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

			void release() override;
		};

		struct DescriptorBindingPrivate : DescriptorBinding
		{
			uint index;
			DescriptorType type;
			uint count;
			const char* name;

			uint get_index() const override;
			DescriptorType get_type() const override;
			uint get_count() const override;
			const char* get_name() const override;
		};

		struct DescriptorlayoutPrivate : Descriptorlayout
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout v;
#elif defined(FLAME_D3D12)

#endif

			std::vector<DescriptorBinding> bindings;
			Descriptorset* default_set;

			uint hash;

			DescriptorlayoutPrivate(DevicePrivate* d, std::span<DescriptorBindingInfo> bindings, bool create_default_set = false);
			~DescriptorlayoutPrivate();

			void release() override;

			uint get_bindings_count() const override;
			DescriptorBinding* get_binding(uint binding) const override;
			Descriptorset* get_default_set() const override;
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate* p;
			DescriptorlayoutPrivate* l;
#if defined(FLAME_VULKAN)
			VkDescriptorSet v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorsetPrivate(DescriptorpoolPrivate* p, DescriptorlayoutPrivate* l);
			~DescriptorsetPrivate();

			void release() override;

			Descriptorlayout* get_layout() const override;

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

			uint hash;

			PipelinelayoutPrivate(DevicePrivate* d, std::span<DescriptorlayoutPrivate*> descriptorlayouts, uint push_constant_size);
			~PipelinelayoutPrivate();

			void release() override;
		};

		struct BlendOptions
		{
			bool enable;
			BlendFactor src_color;
			BlendFactor dst_color;
			BlendFactor src_alpha;
			BlendFactor dst_alpha;

			/*
				if (Enable)
				{
					finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
					finalColor.a   = (srcAlphaBlendFactor * newColor.a  ) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a  );
				}
				else
					finalColor = newColor;

				finalColor = finalColor & colorWriteMask;
			*/

			BlendOptions() :
				enable(false),
				src_color(BlendFactorZero),
				dst_color(BlendFactorZero),
				src_alpha(BlendFactorZero),
				dst_alpha(BlendFactorZero)
			{
			}
		};

		struct ShaderInOut
		{
			std::string name;
			std::string type;
		};

		struct ShaderVariable
		{
			std::string type_name;
			std::string name;
			uint offset;
			uint size;
			uint count;
			uint array_stride;

			std::vector<std::unique_ptr<ShaderVariable>> members;

			ShaderVariable() :
				offset(0),
				size(0),
				count(0),
				array_stride(0)
			{
			}
		};

		struct ShaderResource
		{
			uint location;
			uint index;
			uint set;
			uint binding;
			std::string name;

			ShaderVariable v;

			ShaderResource() :
				location(0),
				index(0),
				set(0),
				binding(0)
			{
			}
		};

		struct ShaderPrivate : Shader
		{
			std::filesystem::path path;
			std::string prefix;
			ShaderStage type;

			std::vector<ShaderInOut> inputs;
			std::vector<ShaderInOut> outputs;
			std::vector<BlendOptions> blend_options;
			std::vector<std::unique_ptr<ShaderResource>> uniform_buffers;
			std::unique_ptr<ShaderResource> push_constant;

#if defined(FLAME_VULKAN)
			VkShaderModule vk_shader_module;
#elif defined(FLAME_D3D12)

#endif
			ShaderPrivate(const std::filesystem::path& fn, const std::string& prefix = "");

			void release() override;

			const wchar_t* get_filename() const override;
			const char* get_prefix() const override;
		};

		struct PipelinePrivate : Pipeline
		{
			PipelineType type;

			DevicePrivate* d;
			PipelinelayoutPrivate* pll;

#if defined(FLAME_VULKAN)
			std::vector<VkShaderModule> vk_shader_modules;
			VkPipeline v;
#elif defined(FLAME_D3D12)

#endif

			PipelinePrivate(DevicePrivate* d, const std::filesystem::path& shader_dir, 
				std::span<ShaderPrivate*> shaders, PipelinelayoutPrivate* pll, RenderpassPrivate* rp, 
				uint subpass_idx, VertexInputInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), 
				RasterInfo* raster = nullptr, SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr, 
				std::span<uint> dynamic_states = {});
			PipelinePrivate(DevicePrivate* d, const std::filesystem::path& shader_dir, ShaderPrivate* compute_shader, PipelinelayoutPrivate* pll);
			~PipelinePrivate();

			void release() override;

			PipelineType get_type() const override;
		};
	}
}
