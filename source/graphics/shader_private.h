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

		struct DescriptorpoolPrivate : Descriptorpool
		{
			DevicePrivate* _d;
#if defined(FLAME_VULKAN)
			VkDescriptorPool _v;
#elif defined(FLAME_D3D12)

#endif
			DescriptorpoolPrivate(DevicePrivate* d);
			~DescriptorpoolPrivate();

			void release() override { delete this; }
		};

		struct DescriptorBindingPrivate : DescriptorBinding
		{
			uint _index;
			DescriptorType _type;
			uint _count;
			std::string _name;

			DescriptorBindingPrivate(uint index, const DescriptorBindingInfo& info);

			uint get_index() const override { return _index; }
			DescriptorType get_type() const override { return _type; }
			uint get_count() const override { return _count; }
			const char* get_name() const override { return _name.c_str(); }
		};

		struct DescriptorlayoutPrivate : Descriptorlayout
		{
			DevicePrivate* _d;
#if defined(FLAME_VULKAN)
			VkDescriptorSetLayout _v;
#elif defined(FLAME_D3D12)

#endif

			std::vector<std::unique_ptr<DescriptorBindingPrivate>> _bindings;
			std::unique_ptr<Descriptorset> _default_set;

			uint _hash;

			DescriptorlayoutPrivate(DevicePrivate* d, std::span<const DescriptorBindingInfo> bindings, bool create_default_set = false);
			~DescriptorlayoutPrivate();

			void release() override { delete this; }

			uint get_bindings_count() const override { return _bindings.size(); }
			DescriptorBinding* get_binding(uint binding) const override { return _bindings[binding].get(); }
			Descriptorset* get_default_set() const override { return _default_set.get(); }
		};

		struct DescriptorsetPrivate : Descriptorset
		{
			DescriptorpoolPrivate* _p;
			DescriptorlayoutPrivate* _l;
#if defined(FLAME_VULKAN)
			VkDescriptorSet _v;
#elif defined(FLAME_D3D12)

#endif

			DescriptorsetPrivate(DescriptorpoolPrivate* p, DescriptorlayoutPrivate* l);
			~DescriptorsetPrivate();

			void release() override { delete this; }

			void _set_buffer(uint binding, uint index, BufferPrivate* b, uint offset = 0, uint range = 0);
			void _set_image(uint binding, uint index, ImageviewPrivate* iv, SamplerPrivate* sampler);

			Descriptorlayout* get_layout() const override { return _l; }

			void set_buffer(uint binding, uint index, Buffer* b, uint offset, uint range) override { _set_buffer(binding, index, (BufferPrivate*)b, offset, range); }
			void set_image(uint binding, uint index, Imageview* v, Sampler* sampler) override { _set_image(binding, index, (ImageviewPrivate*)v, (SamplerPrivate*)sampler); }
		};

		struct PipelinelayoutPrivate : Pipelinelayout
		{
			DevicePrivate* _d;
#if defined(FLAME_VULKAN)
			VkPipelineLayout _v;
#elif defined(FLAME_D3D12)

#endif
			std::vector<DescriptorlayoutPrivate*> _dsls;
			uint _pc_size;

			uint _hash;

			PipelinelayoutPrivate(DevicePrivate* d, std::span<DescriptorlayoutPrivate*> descriptorlayouts, uint push_constant_size);
			~PipelinelayoutPrivate();

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
			std::vector<ShaderInOut> _inputs;
			std::vector<ShaderInOut> _outputs;
			std::vector<BlendOptions> _blend_options;
			std::vector<std::unique_ptr<ShaderResource>> _uniform_buffers;
			std::unique_ptr<ShaderResource> _push_constant;
		};

		struct ShaderPrivate : Shader
		{
			std::filesystem::path _filename;
			std::string _prefix;
			ShaderStage _type;

			ShaderPrivate(const std::filesystem::path& filename, const std::string& prefix = "");

			void release() override { delete this; }

			const wchar_t* get_filename() const override { return _filename.c_str(); }
			const char* get_prefix() const override { return _prefix.c_str(); }
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
			PipelineType _type;

			DevicePrivate* _d;
			PipelinelayoutPrivate* _pll;
			std::vector<CompiledShader> _shaders;
#if defined(FLAME_VULKAN)
			VkPipeline _v;
#elif defined(FLAME_D3D12)

#endif

			PipelinePrivate(DevicePrivate* d, std::span<CompiledShader> shaders, PipelinelayoutPrivate* pll, RenderpassPrivate* rp,
				uint subpass_idx, VertexInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), RasterInfo* raster = nullptr, 
				SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr, std::span<const uint> dynamic_states = {});
			PipelinePrivate(DevicePrivate* d, CompiledShader& compute_shader, PipelinelayoutPrivate* pll);
			~PipelinePrivate();

			static PipelinePrivate* _create(DevicePrivate* d, const std::filesystem::path& shader_dir, std::span<ShaderPrivate*> shaders, 
				PipelinelayoutPrivate* pll, Renderpass* rp, uint subpass_idx, VertexInfo* vi = nullptr, const Vec2u& vp = Vec2u(0), 
				RasterInfo* raster = nullptr, SampleCount sc = SampleCount_1, DepthInfo* depth = nullptr, std::span<const uint> dynamic_states = {});
			static PipelinePrivate* _create(DevicePrivate* d, const std::filesystem::path& shader_dir, ShaderPrivate* compute_shader, PipelinelayoutPrivate* pll);

			void release() override { delete this; }

			PipelineType get_type() const override { return _type; }
		};
	}
}
