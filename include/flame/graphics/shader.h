#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Renderpass;
		struct Buffer;
		struct Imageview;
		struct Sampler;

		struct Descriptorpool
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorpool* create(Device* d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorpool* p);
		};

		struct DescriptorsetBinding
		{
			uint binding;
			DescriptorType type;
			uint count;

			DescriptorsetBinding(uint binding, DescriptorType type, uint count = 1) :
				binding(binding),
				type(type),
				count(count)
			{
			}
		};

		struct Descriptorsetlayout
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorsetlayout* create(Device* d, const std::vector<DescriptorsetBinding>& bindings);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorsetlayout* l);
		};

		struct Descriptorset
		{
			FLAME_GRAPHICS_EXPORTS void set_uniformbuffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_storagebuffer(uint binding, uint index, Buffer* b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_imageview(uint binding, uint index, Imageview* v, Sampler* sampler);
			FLAME_GRAPHICS_EXPORTS void set_storageimage(uint binding, uint index, Imageview* v);

			FLAME_GRAPHICS_EXPORTS static Descriptorset* create(Descriptorpool* p, Descriptorsetlayout* l);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorset* s);
		};

		struct Shader
		{
			ShaderType$ type;

			FLAME_GRAPHICS_EXPORTS static Shader* create(Device *d, const std::wstring &filename, const std::string &prefix);
			FLAME_GRAPHICS_EXPORTS static void destroy(Shader *s);
		};

		struct Pipelinelayout
		{
			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, const std::vector<void*>& descriptorsetlayouts, uint push_constant_size);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout* p);
		};

		enum VertexInputRate
		{
			VertexInputRateVertex,
			VertexInputRateInstance,
		};

		struct VertexInputAttributeInfo
		{
			uint location;
			uint buffer_id;
			uint offset;
			Format$ format;

			VertexInputAttributeInfo(uint location, uint buffer_id, uint offset, Format$ format) :
				location(location),
				buffer_id(buffer_id),
				offset(offset),
				format(format)
			{
			}
		};

		struct VertexInputBufferInfo
		{
			uint id;
			uint stride;
			VertexInputRate rate;

			VertexInputBufferInfo(uint id, uint stride, VertexInputRate rate = VertexInputRateVertex) :
				id(id),
				stride(stride),
				rate(rate)
			{
			}
		};

		enum PrimitiveTopology
		{
			PrimitiveTopologyPointList,
			PrimitiveTopologyLineList,
			PrimitiveTopologyLineStrip,
			PrimitiveTopologyTriangleList,
			PrimitiveTopologyTriangleStrip,
			PrimitiveTopologyTriangleFan,
			PrimitiveTopologyLineListWithAdjacency,
			PrimitiveTopologyLineStripWithAdjacency,
			PrimitiveTopologyTriangleListWithAdjacency,
			PrimitiveTopologyTriangleStripWithAdjacency,
			PrimitiveTopologyPatchList,
		};

		enum PolygonMode
		{
			PolygonModeFill,
			PolygonModeLine,
			PolygonModePoint
		};

		enum CompareOp
		{
			CompareOpLess,
			CompareOpLessOrEqual,
			CompareOpGreater,
			CompareOpGreaterOrEqual,
			CompareOpEqual,
			CompareOpNotEqual,
			CompareOpAlways,
		};

		enum CullMode
		{
			CullModeNone,
			CullModeFront,
			CullModeBack,
			CullModeFrontAndback,
		};

		enum BlendFactor
		{
			BlendFactorZero,
			BlendFactorOne,
			BlendFactorSrcColor,
			BlendFactorOneMinusSrcColor,
			BlendFactorDstColor,
			BlendFactorOneMinusDstColor,
			BlendFactorSrcAlpha,
			BlendFactorOneMinusSrcAlpha,
			BlendFactorDstAlpha,
			BlendFactorOneMinusDstAlpha,
			BlendFactorConstantColor,
			BlendFactorOneMinusConstantColor,
			BlendFactorConstantAlpha,
			BlendFactorOneMinusConstantAlpha,
			BlendFactorSrcAlphaSaturate,
			BlendFactorSrc1Color,
			BlendFactorOneMinusSrc1Color,
			BlendFactorSrc1Alpha,
			BlendFactorOneMinusSrc1Alpha
		};

		struct BlendInfo
		{
			bool blend_enable;
			BlendFactor blend_src_color;
			BlendFactor blend_dst_color;
			BlendFactor blend_src_alpha;
			BlendFactor blend_dst_alpha;

			BlendInfo()
			{
				blend_enable = false;
				blend_src_color = BlendFactorOne;
				blend_dst_color = BlendFactorZero;
				blend_src_alpha = BlendFactorOne;
				blend_dst_alpha = BlendFactorZero;
			}

			BlendInfo(BlendFactor bsc, BlendFactor bdc, BlendFactor bsa, BlendFactor bda)
			{
				blend_enable = true;
				blend_src_color = bsc;
				blend_dst_color = bdc;
				blend_src_alpha = bsa;
				blend_dst_alpha = bda;
			}
		};

		enum DynamicState
		{
			DynamicStateViewport,
			DynamicStateScissor,
			DynamicStateLineWidth,
			DynamicStateDepthBias,
			DynamicStateBlendConstants,
			DynamicStateDepthBounds,
			DynamicStateStencilCompareMask,
			DynamicStateStencilWriteMask,
			DynamicStateStencilReference
		};

		struct GraphicsPipelineInfo
		{
			std::vector<Shader*> shaders;
			Pipelinelayout* layout;
			std::vector<VertexInputAttributeInfo> vi_attribs;
			std::vector<VertexInputBufferInfo> vi_buffers;
			PrimitiveTopology primitive_topology;
			uint patch_control_points;
			Vec2u viewport_size;
			bool depth_clamp;
			PolygonMode polygon_mode;
			CullMode cull_mode;
			SampleCount$ sample_count;
			bool depth_test;
			bool depth_write;
			CompareOp depth_compare_op;
			std::vector<BlendInfo> blend_states;
			std::vector<DynamicState> dynamic_states;
			Renderpass* renderpass;
			uint subpass_idx;

			GraphicsPipelineInfo(Pipelinelayout* layout, Renderpass* rp, uint subpass_idx) :
				layout(layout),
				primitive_topology(PrimitiveTopologyTriangleList),
				patch_control_points(0),
				viewport_size(0),
				depth_clamp(false),
				polygon_mode(PolygonModeFill),
				cull_mode(CullModeBack),
				sample_count(SampleCount_1),
				depth_test(false),
				depth_write(false),
				depth_compare_op(CompareOpLess),
				renderpass(rp),
				subpass_idx(subpass_idx)
			{
				blend_states.resize(renderpass->subpass_col_ref_count(subpass_idx));
			}
		};

		struct ComputePipelineInfo
		{
			Shader* compute_shader;
			Pipelinelayout* layout;

			ComputePipelineInfo(Shader* compute_shader, Pipelinelayout* layout) :
				compute_shader(compute_shader),
				layout(layout)
			{
			}
		};

		struct Pipeline
		{
			PipelineType type;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const GraphicsPipelineInfo& info);
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const ComputePipelineInfo& info);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline* p);
		};
	}
}
