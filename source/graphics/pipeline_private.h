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

#include <flame/graphics/pipeline.h>
#include "graphics_private.h"

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct RenderpassPrivate;
		struct ShaderPrivate;

		inline VkVertexInputRate Z(VertexInputRate r)
		{
			switch (r)
			{
			case VertexInputRateVertex:
				return VK_VERTEX_INPUT_RATE_VERTEX;
			case VertexInputRateInstance:
				return VK_VERTEX_INPUT_RATE_INSTANCE;
			}
		}

		inline VkPrimitiveTopology Z(PrimitiveTopology t)
		{
			switch (t)
			{
				case PrimitiveTopologyPointList:
					return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
				case PrimitiveTopologyLineList:
					return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				case PrimitiveTopologyLineStrip:
					return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
				case PrimitiveTopologyTriangleList:
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				case PrimitiveTopologyTriangleStrip:
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				case PrimitiveTopologyTriangleFan:
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
				case PrimitiveTopologyLineListWithAdjacency:
					return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
				case PrimitiveTopologyLineStripWithAdjacency:
					return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
				case PrimitiveTopologyTriangleListWithAdjacency:
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
				case PrimitiveTopologyTriangleStripWithAdjacency:
					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
				case PrimitiveTopologyPatchList:
					return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			}
		}

		inline VkPolygonMode Z(PolygonMode m)
		{
			switch (m)
			{
				case PolygonModeFill:
					return VK_POLYGON_MODE_FILL;
				case PolygonModeLine:
					return VK_POLYGON_MODE_LINE;
				case PolygonModePoint:
					return VK_POLYGON_MODE_POINT;
			}
		}

		inline VkCompareOp Z(CompareOp o)
		{
			switch (o)
			{
			case CompareOpLess:
				return VK_COMPARE_OP_LESS;
			case CompareOpLessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOpGreater:
				return VK_COMPARE_OP_GREATER;
			case CompareOpGreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOpEqual:
				return VK_COMPARE_OP_EQUAL;
			case CompareOpNotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
			case CompareOpAlways:
				return VK_COMPARE_OP_ALWAYS;
			}
		}

		inline VkCullModeFlagBits Z(CullMode m)
		{
			switch (m)
			{
				case CullModeNone:
					return VK_CULL_MODE_NONE;
				case CullModeFront:
					return VK_CULL_MODE_FRONT_BIT;
				case CullModeBack:
					return VK_CULL_MODE_BACK_BIT;
				case CullModeFrontAndback:
					return VK_CULL_MODE_FRONT_AND_BACK;
			}
		}

		inline VkBlendFactor Z(BlendFactor f)
		{
			switch (f)
			{
				case BlendFactorZero:
					return VK_BLEND_FACTOR_ZERO;
				case BlendFactorOne:
					return VK_BLEND_FACTOR_ONE;
				case BlendFactorSrcColor:
					return VK_BLEND_FACTOR_SRC_COLOR;
				case BlendFactorOneMinusSrcColor:
					return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
				case BlendFactorDstColor:
					return VK_BLEND_FACTOR_DST_COLOR;
				case BlendFactorOneMinusDstColor:
					return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
				case BlendFactorSrcAlpha:
					return VK_BLEND_FACTOR_SRC_ALPHA;
				case BlendFactorOneMinusSrcAlpha:
					return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				case BlendFactorDstAlpha:
					return VK_BLEND_FACTOR_DST_ALPHA;
				case BlendFactorOneMinusDstAlpha:
					return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				case BlendFactorConstantColor:
					return VK_BLEND_FACTOR_CONSTANT_COLOR;
				case BlendFactorOneMinusConstantColor:
					return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
				case BlendFactorConstantAlpha:
					return VK_BLEND_FACTOR_CONSTANT_ALPHA;
				case BlendFactorOneMinusConstantAlpha:
					return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
				case BlendFactorSrcAlphaSaturate:
					return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
				case BlendFactorSrc1Color:
					return VK_BLEND_FACTOR_SRC1_COLOR;
				case BlendFactorOneMinusSrc1Color:
					return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
				case BlendFactorSrc1Alpha:
					return VK_BLEND_FACTOR_SRC1_ALPHA;
				case BlendFactorOneMinusSrc1Alpha:
					return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
			}
		}

		inline VkDynamicState Z(DynamicState s)
		{
			switch (s)
			{
				case DynamicStateViewport:
					return VK_DYNAMIC_STATE_VIEWPORT;
				case DynamicStateScissor:
					return VK_DYNAMIC_STATE_SCISSOR;
				case DynamicStateLineWidth:
					return VK_DYNAMIC_STATE_LINE_WIDTH;
				case DynamicStateDepthBias:
					return VK_DYNAMIC_STATE_DEPTH_BIAS;
				case DynamicStateBlendConstants:
					return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
				case DynamicStateDepthBounds:
					return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
				case DynamicStateStencilCompareMask:
					return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
				case DynamicStateStencilWriteMask:
					return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
				case DynamicStateStencilReference:
					return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
			}
		}

		struct PipelinelayoutPrivate : Pipelinelayout
		{
			DevicePrivate *d;
			std::vector<Descriptorsetlayout*> dsls;
			std::vector<PushconstantInfo> pcs;
			VkPipelineLayout v;

			int ref_count;

			PipelinelayoutPrivate(Device *d, const std::vector<Descriptorsetlayout*> &_setlayouts, const std::vector<PushconstantInfo> &_pushconstants);
			~PipelinelayoutPrivate();
		};

		struct PipelinePrivate : Pipeline
		{
			DevicePrivate *d;

			ShaderPrivate *vert_shader;
			ShaderPrivate *tesc_shader;
			ShaderPrivate *tese_shader;
			ShaderPrivate *geom_shader;
			ShaderPrivate *frag_shader;
			ShaderPrivate *comp_shader;
			std::vector<Descriptorsetlayout*> dsls;
			PipelinelayoutPrivate *layout;
			VkPipeline v;

			void init();

			PipelinePrivate(Device *d, const GraphicsPipelineInfo &info);
			PipelinePrivate(Device *d, const ShaderInfo &compute_shader);
			~PipelinePrivate();

			void add_shader(const ShaderInfo &info);
			std::vector<VkPipelineShaderStageCreateInfo> process_stages();
		};
	}
}
