#pragma once

#ifdef FLAME_WINDOWS
#ifdef FLAME_GRAPHICS_MODULE
#define FLAME_GRAPHICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_GRAPHICS_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_GRAPHICS_EXPORTS
#endif

#include <flame/math.h>

namespace flame
{
	namespace graphics
	{
		enum Format$
		{
			Format_Undefined,

			Format_R8_UNORM,
			Format_R16_UNORM,
			Format_R32_SFLOAT,
			Format_R32G32_SFLOAT,
			Format_R32G32B32_SFLOAT,
			Format_R8G8B8A8_UNORM,
			Format_R8G8B8A8_SRGB,
			Format_B8G8R8A8_UNORM,
			Format_B8G8R8A8_SRGB,
			Format_Swapchain_B8G8R8A8_UNORM,
			Format_Swapchain_B8G8R8A8_SRGB,
			Format_Swapchain_Begin = Format_Swapchain_B8G8R8A8_UNORM,
			Format_Swapchain_End = Format_Swapchain_B8G8R8A8_SRGB,
			Format_R16G16B16A16_UNORM,
			Format_R16G16B16A16_SFLOAT,
			Format_R32G32B32A32_SFLOAT,
			Format_RGBA_BC3,
			Format_RGBA_ETC2,
			Format_Color_Begin = Format_R8_UNORM,
			Format_Color_End = Format_RGBA_ETC2,

			Format_Depth16,
			Format_Depth32,
			Format_Depth24Stencil8,
			Format_DepthStencil_Begin = Format_Depth24Stencil8,
			Format_DepthStencil_End = Format_Depth24Stencil8,
			Format_Depth_Begin = Format_Depth16,
			Format_Depth_End = Format_Depth24Stencil8,
		};

		enum MemProp$
		{
			MemPropDevice = 1 << 0,
			MemPropHost = 1 << 1,
			MemPropHostCoherent = 1 << 2
		};

		enum DescriptorSetBindings
		{
			MainDescriptorSetBinding,
			MaterialDescriptorSetBinding,
			BoneSetDescriptorBinding
		};

		enum SampleCount$
		{
			SampleCount_1,
			SampleCount_2,
			SampleCount_4,
			SampleCount_8,
			SampleCount_16,
			SampleCount_32
		};

		enum ShaderStage$
		{
			ShaderStageNone,
			ShaderStageVert = 1 << 0,
			ShaderStageTesc = 1 << 1,
			ShaderStageTese = 1 << 2,
			ShaderStageGeom = 1 << 3,
			ShaderStageFrag = 1 << 4,
			ShaderStageComp = 1 << 5,
			ShaderStageAll = ShaderStageVert | ShaderStageTesc | ShaderStageTese | ShaderStageGeom | ShaderStageFrag
		};

		enum DescriptorType$
		{
			DescriptorUniformBuffer,
			DescriptorStorageBuffer,
			DescriptorSampledImage,
			DescriptorStorageImage
		};

		enum PipelineType
		{
			PipelineNone,
			PipelineGraphics,
			PipelineCompute
		};

		enum BufferUsage$
		{
			BufferUsageTransferSrc = 1 << 0,
			BufferUsageTransferDst = 1 << 1,
			BufferUsageUniform = 1 << 2,
			BufferUsageStorage = 1 << 3,
			BufferUsageIndex = 1 << 4,
			BufferUsageVertex = 1 << 5,
			BufferUsageIndirect = 1 << 6
		};

		enum ImageUsage$
		{
			ImageUsageTransferSrc = 1 << 0,
			ImageUsageTransferDst = 1 << 1,
			ImageUsageSampled = 1 << 2,
			ImageUsageStorage = 1 << 3,
			ImageUsageAttachment = 1 << 4
		};

		enum ImageLayout
		{
			ImageLayoutUndefined,
			ImageLayoutAttachment,
			ImageLayoutShaderReadOnly,
			ImageLayoutShaderStorage,
			ImageLayoutTransferSrc,
			ImageLayoutTransferDst,
			ImageLayoutPresent
		};

		enum ImageAspect
		{
			ImageAspectColor = 1 << 0,
			ImageAspectDepth = 1 << 1,
			ImageAspectStencil = 1 << 2
		};

		enum ImageviewType$
		{
			Imageview1D,
			Imageview2D,
			Imageview3D,
			ImageviewCube,
			Imageview1DArray,
			ImageView2DArray,
			ImageViewCubeArray
		};

		enum Swizzle$
		{
			SwizzleIdentity,
			SwizzleZero,
			SwizzleOne,
			SwizzleR,
			SwizzleG,
			SwizzleB,
			SwizzleA
		};

		enum IndiceType
		{
			IndiceTypeUint,
			IndiceTypeUshort
		};

		enum Filter
		{
			FilterNearest,
			FilterLinear
		};

		enum VertexInputRate
		{
			VertexInputRateVertex,
			VertexInputRateInstance,
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
	}
}
