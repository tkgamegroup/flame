 #pragma once

#ifdef FLAME_GRAPHICS_MODULE
#define FLAME_GRAPHICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_GRAPHICS_EXPORTS __declspec(dllimport)
#endif

#include <flame/math.h>

namespace flame
{
	namespace graphics
	{
		enum Format
		{
			Format_Undefined,

			Format_R8_UNORM,
			Format_R16_UNORM,
			Format_R16_SFLOAT,
			Format_R32_UINT,
			Format_R32_SFLOAT,
			Format_R32G32_SFLOAT,
			Format_R32G32B32_SFLOAT,
			Format_R8G8B8A8_UNORM,
			Format_R8G8B8A8_SRGB,
			Format_B8G8R8A8_UNORM,
			Format_B8G8R8A8_SRGB,
			Format_R16G16B16A16_UNORM,
			Format_R16G16B16A16_SFLOAT,
			Format_R32G32B32A32_SFLOAT,
			Format_R32G32B32A32_INT,
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

			Format_Max = 0xffffffff
		};

		enum MemoryPropertyFlags
		{
			MemoryPropertyDevice = 1 << 0,
			MemoryPropertyHost = 1 << 1,
			MemoryPropertyCoherent = 1 << 2,

			MemoryPropertyMax = 0xffffffff
		};

		inline MemoryPropertyFlags operator| (MemoryPropertyFlags a, MemoryPropertyFlags b) { return (MemoryPropertyFlags)((int)a | (int)b); }

		enum SampleCount
		{
			SampleCount_1,
			SampleCount_2,
			SampleCount_4,
			SampleCount_8,
			SampleCount_16,
			SampleCount_32,

			SampleCountMax = 0xffffffff
		};

		enum BufferUsageFlags
		{
			BufferUsageNone,
			BufferUsageTransferSrc = 1 << 0,
			BufferUsageTransferDst = 1 << 1,
			BufferUsageUniform = 1 << 2,
			BufferUsageStorage = 1 << 3,
			BufferUsageIndex = 1 << 4,
			BufferUsageVertex = 1 << 5,
			BufferUsageIndirect = 1 << 6,

			BufferUsageMax = 0xffffffff
		};

		inline BufferUsageFlags operator| (BufferUsageFlags a, BufferUsageFlags b) { return (BufferUsageFlags)((int)a | (int)b); }

		enum ImageUsageFlags
		{
			ImageUsageNone,
			ImageUsageTransferSrc = 1 << 0,
			ImageUsageTransferDst = 1 << 1,
			ImageUsageSampled = 1 << 2,
			ImageUsageStorage = 1 << 3,
			ImageUsageAttachment = 1 << 4,

			ImageUsageMax = 0xffffffff
		};

		inline ImageUsageFlags operator| (ImageUsageFlags a, ImageUsageFlags b) { return (ImageUsageFlags)((int)a | (int)b); }

		enum ImageLayout
		{
			ImageLayoutUndefined,
			ImageLayoutAttachment,
			ImageLayoutShaderReadOnly,
			ImageLayoutShaderStorage,
			ImageLayoutTransferSrc,
			ImageLayoutTransferDst,
			ImageLayoutPresent,
			ImageLayoutGeneral
		};

		enum ImageAspectFlags
		{
			ImageAspectColor = 1 << 0,
			ImageAspectDepth = 1 << 1,
			ImageAspectStencil = 1 << 2
		};

		inline ImageAspectFlags operator| (ImageAspectFlags a, ImageAspectFlags b) { return (ImageAspectFlags)((int)a | (int)b); }

		enum ImageViewType
		{
			ImageView1D,
			ImageView2D,
			ImageView3D,
			ImageViewCube,
			ImageView1DArray,
			ImageView2DArray,
			ImageViewCubeArray,

			ImageViewMax = 0xffffffff
		};

		struct ImageSubresource
		{
			uint base_level = 0;
			uint level_count = 1;
			uint base_layer = 0;
			uint layer_count = 1;
		};

		enum Swizzle
		{
			SwizzleIdentity,
			SwizzleZero,
			SwizzleOne,
			SwizzleR,
			SwizzleG,
			SwizzleB,
			SwizzleA
		};

		struct ImageSwizzle
		{
			Swizzle r = SwizzleIdentity;
			Swizzle g = SwizzleIdentity;
			Swizzle b = SwizzleIdentity;
			Swizzle a = SwizzleIdentity;
		};

		enum AddressMode
		{
			AddressClampToEdge,
			AddressClampToBorder,
			AddressRepeat
		};

		enum Filter
		{
			FilterNearest,
			FilterLinear
		};

		enum AccessFlags
		{
			AccessNone = 0,
			AccessIndirectCommandRead = 1 << 0,
			AccessIndexRead = 1 << 1,
			AccessVertexAttributeRead = 1 << 2,
			AccessUniformRead = 1 << 3,
			AccessInputAttachmentRead = 1 << 4,
			AccessShaderRead = 1 << 5,
			AccessShaderWrite = 1 << 6,
			AccessColorAttachmentRead = 1 << 7,
			AccessColorAttachmentWrite = 1 << 8,
			AccessDepthAttachmentRead = 1 << 9,
			AccessDepthAttachmentWrite = 1 << 10,
			AccessTransferRead = 1 << 11,
			AccessTransferWrite = 1 << 12,
			AccessHostRead = 1 << 13,
			AccessHostWrite = 1 << 14,
			AccessMemoryRead = 1 << 15,
			AccessMemoryWrite = 1 << 16
		};

		inline AccessFlags operator| (AccessFlags a, AccessFlags b) { return (AccessFlags)((int)a | (int)b); }

		enum AttachmentLoadOp
		{
			AttachmentLoad,
			AttachmentClear,
			AttachmentDontCare
		};

		enum QueueFamily
		{
			QueueGraphics,
			QueueTransfer
		};

		enum DescriptorType
		{
			DescriptorUniformBuffer,
			DescriptorStorageBuffer,
			DescriptorSampledImage,
			DescriptorStorageImage,

			DescriptorMax = 0xffffffff
		};

		enum PipelineType
		{
			PipelineNone,
			PipelineGraphics,
			PipelineCompute
		};

		enum IndiceType
		{
			IndiceTypeUint,
			IndiceTypeUshort
		};

		enum VertexInputRate
		{
			VertexInputRateVertex,
			VertexInputRateInstance
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
			PrimitiveTopologyPatchList
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
			CompareOpAlways
		};

		enum CullMode
		{
			CullModeNone,
			CullModeFront,
			CullModeBack,
			CullModeFrontAndback
		};

		enum ShaderStageFlags
		{
			ShaderStageNone,
			ShaderStageVert = 1 << 0,
			ShaderStageTesc = 1 << 1,
			ShaderStageTese = 1 << 2,
			ShaderStageGeom = 1 << 3,
			ShaderStageFrag = 1 << 4,
			ShaderStageComp = 1 << 5,
			ShaderStageAll = ShaderStageVert | ShaderStageTesc | ShaderStageTese | ShaderStageGeom | ShaderStageFrag | ShaderStageComp,

			ShaderStageMax = 0xffffffff
		};

		inline ShaderStageFlags operator| (ShaderStageFlags a, ShaderStageFlags b) { return (ShaderStageFlags)((int)a | (int)b); }

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

		inline bool is_blend_factor_dual(BlendFactor f)
		{
			switch (f)
			{
			case BlendFactorSrc1Color:
			case BlendFactorOneMinusSrc1Color:
			case BlendFactorSrc1Alpha:
			case BlendFactorOneMinusSrc1Alpha:
				return true;

			}
			return false;
		}

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

		enum LightType
		{
			LightDirectional,
			LightPoint,
			LightSpot
		};
	}
}
