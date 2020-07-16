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

		struct DescriptorPoolPrivate : DescriptorPool
		{
			DevicePrivate* device;
#if defined(FLAME_VULKAN)
			VkDescriptorPool vk_descriptor_pool;
#elif defined(FLAME_D3D12)

#endif
			DescriptorPoolPrivate(DevicePrivate* d);
			~DescriptorPoolPrivate();

			void release() override { delete this; }
		};

		struct DescriptorBindingPrivate : DescriptorBinding
		{
			uint index;
			DescriptorType type;
			uint count;
			std::string name;

			DescriptorBindingPrivate(uint index, const DescriptorBindingInfo& info);

			uint get_index() const override { return index; }
			DescriptorType get_type() const override { return type; }
			uint get_count() const override { return count; }
			const char* get_name() const override { return name.c_str(); }
		};

		struct DescriptorSetLayoutPrivate : DescriptorSetLayout
		{
			DevicePrivate* device;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout vk_descriptor_set_layout;
#elif defined(FLAME_D3D12)

#endif

			std::vector<std::unique_ptr<DescriptorBindingPrivate>> bindings;
			std::unique_ptr<DescriptorSetPrivate> default_set;

			uint64 hash;

			DescriptorSetLayoutPrivate(DevicePrivate* d, std::span<const DescriptorBindingInfo> bindings, bool create_default_set = false);
			~DescriptorSetLayoutPrivate();

			void release() override { delete this; }

			uint get_bindings_count() const override { return bindings.size(); }
			DescriptorBinding* get_binding(uint binding) const override { return bindings[binding].get(); }
			DescriptorSet* get_default_set() const override { return (DescriptorSet*)default_set.get(); }
		};

		struct DescriptorSetBridge : DescriptorSet
		{
			void set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range) override;
			void set_image(uint binding, uint index, ImageView* v, Sampler* sampler) override;
		};

		struct DescriptorSetPrivate : DescriptorSetBridge
		{
			DescriptorPoolPrivate* descriptor_pool;
			DescriptorSetLayoutPrivate* descriptor_layout;
#if defined(FLAME_VULKAN)
			VkDescriptorSet vk_descriptor_set;
#elif defined(FLAME_D3D12)

#endif

			DescriptorSetPrivate(DescriptorPoolPrivate* p, DescriptorSetLayoutPrivate* l);
			~DescriptorSetPrivate();

			void release() override { delete this; }

			DescriptorSetLayout* get_layout() const override { return descriptor_layout; }

			void set_buffer(uint binding, uint index, BufferPrivate* b, uint offset = 0, uint range = 0);
			void set_image(uint binding, uint index, ImageViewPrivate* iv, SamplerPrivate* sampler);
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
#if defined(FLAME_VULKAN)
			VkPipelineLayout vk_pipeline_layout;
#elif defined(FLAME_D3D12)

#endif
			std::vector<DescriptorSetLayoutPrivate*> descriptor_layouts;
			uint push_cconstant_size;

			uint64 hash;

			PipelineLayoutPrivate(DevicePrivate* d, std::span<DescriptorSetLayoutPrivate*> descriptorlayouts, uint push_constant_size);
			~PipelineLayoutPrivate();

			void release() override { delete this; }
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
			uint offset = 0;
			uint size = 0;
			uint count = 0;
			uint array_stride = 0;

			std::vector<std::unique_ptr<ShaderVariable>> members;
		};

		struct ShaderBindng
		{
			uint location = 0;
			uint index = 0;
			uint set = 0;
			uint binding = 0;
			std::string name;

			ShaderVariable v;
		};

		struct ShaderResource
		{
			std::vector<ShaderInOut> inputs;
			std::vector<ShaderInOut> outputs;
			std::vector<BlendOptions> blend_options;
			std::vector<std::unique_ptr<ShaderResource>> uniform_buffers;
			std::unique_ptr<ShaderResource> push_constant;
		};

		struct ShaderPrivate : Shader
		{
			std::filesystem::path filename;
			std::string prefix;
			ShaderStageFlags type;

			ShaderPrivate(const std::filesystem::path& filename, const std::string& prefix = "");

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return filename.c_str(); }
			const char* get_prefix() const override { return prefix.c_str(); }
		};

		struct CompiledShader
		{
			ShaderPrivate* s;
			ShaderResource r;
#if defined(FLAME_VULKAN)
			VkShaderModule m;
#elif defined(FLAME_D3D12)

#endif
		};

		struct PipelinePrivate : Pipeline
		{
			PipelineType type;

			DevicePrivate* device;
			PipelineLayoutPrivate* pipeline_layout;
			std::vector<CompiledShader> shaders;
#if defined(FLAME_VULKAN)
			VkPipeline vk_pipeline;
#elif defined(FLAME_D3D12)

#endif

			PipelinePrivate(DevicePrivate* d, std::vector<CompiledShader>& shaders, PipelineLayoutPrivate* pll, RenderpassPrivate* rp,
				uint subpass_idx, VertexInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, 
				SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr, std::span<const uint> dynamic_states = {});
			PipelinePrivate(DevicePrivate* d, CompiledShader& compute_shader, PipelineLayoutPrivate* pll);
			~PipelinePrivate();

			static PipelinePrivate* create(DevicePrivate* d, const std::filesystem::path& shader_dir, std::span<ShaderPrivate*> shaders, 
				PipelineLayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInfo* vi = nullptr, const Vec2u& vp = Vec2u(0),
				RasterInfo* raster = nullptr, SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr, std::span<const uint> dynamic_states = {});
			static PipelinePrivate* create(DevicePrivate* d, const std::filesystem::path& shader_dir, ShaderPrivate* compute_shader, PipelineLayoutPrivate* pll);

			void release() override { delete this; }

			PipelineType get_type() const override { return type; }
		};
	}
}
