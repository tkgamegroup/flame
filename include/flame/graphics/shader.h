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

		struct Descriptorsetlayout
		{
			struct Binding
			{
				uint binding;
				ShaderResourceType type;
				uint count;
			};

			FLAME_GRAPHICS_EXPORTS static Descriptorsetlayout* create(Device* d, const std::vector<Binding>& bindings);
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
			FLAME_GRAPHICS_EXPORTS Descriptorsetlayout* dsl(uint index) const;

			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device* d, const std::vector<Descriptorsetlayout*>& setlayouts, uint push_constant_size);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout* p);
		};

		struct ShaderInfo
		{
			std::wstring filename;
			std::string prefix;

			ShaderInfo()
			{
			}

			ShaderInfo(const std::wstring& _filename, const std::string& _prefix) :
				filename(_filename),
				prefix(_prefix)
			{
			}
		};

		enum VertexInputRate
		{
			VertexInputRateVertex,
			VertexInputRateInstance,
		};

		struct VertexInputBufferInfo
		{
			std::vector<Format$> attributes;
			VertexInputRate rate;

			VertexInputBufferInfo() :
				rate(VertexInputRateVertex)
			{
			}

			VertexInputBufferInfo(const std::vector<Format$>& _attributes, VertexInputRate _rate = VertexInputRateVertex) :
				attributes(_attributes),
				rate(_rate)
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
			// stages
			std::vector<ShaderInfo> shaders;

			// vertex input
			std::vector<VertexInputBufferInfo> vi_buffers;

			// vertex assembly
			PrimitiveTopology primitive_topology;

			// tessellation
			int patch_control_points;

			// viewport
			Vec2u viewport_size;

			// raster
			bool depth_clamp;
			PolygonMode polygon_mode;
			CullMode cull_mode;

			// multisample
			SampleCount$ sample_count;

			// depth stencil
			bool depth_test;
			bool depth_write;
			CompareOp depth_compare_op;

			// blend
			std::vector<BlendInfo> blend_states;

			// dynamic
			std::vector<DynamicState> dynamic_states;

			// renderpass
			Renderpass* renderpass;
			int subpass_index;

			GraphicsPipelineInfo() :
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
				renderpass(nullptr),
				subpass_index(0)
			{
				blend_states.push_back(BlendInfo());
			}
		};

		struct Pipeline
		{
			PipelineType type;

			FLAME_GRAPHICS_EXPORTS Pipelinelayout* layout() const;

			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const GraphicsPipelineInfo& info);
			FLAME_GRAPHICS_EXPORTS static Pipeline* create(Device* d, const ShaderInfo& compute_shader);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline* p);
		};
	}
}
