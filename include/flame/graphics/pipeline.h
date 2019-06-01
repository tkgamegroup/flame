// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <flame/graphics/graphics.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Renderpass;
		struct Descriptorsetlayout;

		struct PushconstantInfo
		{
			int offset;
			int size;
		};

		struct Pipelinelayout
		{
			FLAME_GRAPHICS_EXPORTS Descriptorsetlayout *dsl(int index) const;

			FLAME_GRAPHICS_EXPORTS static Pipelinelayout* create(Device *d, const std::vector<Descriptorsetlayout*> &setlayouts, const std::vector<PushconstantInfo> &pushconstants);
			FLAME_GRAPHICS_EXPORTS static void destroy(Pipelinelayout *p);
		};

		struct ShaderInfo
		{
			std::wstring filename;
			std::string prefix;

			ShaderInfo()
			{
			}

			ShaderInfo(const std::wstring &_filename, const std::string &_prefix) : 
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
			std::vector<Format> attributes;
			VertexInputRate rate;

			VertexInputBufferInfo() :
				rate(VertexInputRateVertex)
			{
			}

			VertexInputBufferInfo(const std::vector<Format> &_attributes, VertexInputRate _rate = VertexInputRateVertex) :
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
			SampleCount sample_count;

			// depth stencil
			bool depth_test;
			bool depth_write;
			CompareOp depth_compare_op;

			// blend
			std::vector<BlendInfo> blend_states;

			// dynamic
			std::vector<DynamicState> dynamic_states;

			// renderpass
			Renderpass *renderpass;
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

			FLAME_GRAPHICS_EXPORTS Pipelinelayout *layout() const;

			FLAME_GRAPHICS_EXPORTS static Pipeline *create(Device *d, const GraphicsPipelineInfo &info);
			FLAME_GRAPHICS_EXPORTS static Pipeline *create(Device *d, const ShaderInfo &compute_shader);
			FLAME_GRAPHICS_EXPORTS static  void destroy(Pipeline *p);
		};
	}
}
