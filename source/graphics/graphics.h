 #pragma once

#include "../foundation/foundation.h"

#ifdef FLAME_GRAPHICS_MODULE

#define FLAME_GRAPHICS_API __declspec(dllexport)

#define FLAME_GRAPHICS_TYPE(name) FLAME_TYPE_PRIVATE(name)

#else

#define FLAME_GRAPHICS_API __declspec(dllimport)

#define FLAME_GRAPHICS_TYPE(name) FLAME_TYPE(name)

#endif

namespace flame
{
	namespace graphics
	{
		FLAME_GRAPHICS_TYPE(Device);
		FLAME_GRAPHICS_TYPE(Buffer);
		FLAME_GRAPHICS_TYPE(Image);
		FLAME_GRAPHICS_TYPE(ImageView);
		FLAME_GRAPHICS_TYPE(CommandPool);
		FLAME_GRAPHICS_TYPE(CommandBuffer);
		FLAME_GRAPHICS_TYPE(Semaphore);
		FLAME_GRAPHICS_TYPE(Event);
		FLAME_GRAPHICS_TYPE(Fence);
		FLAME_GRAPHICS_TYPE(Queue);
		FLAME_GRAPHICS_TYPE(Shader);
		FLAME_GRAPHICS_TYPE(DescriptorPool);
		FLAME_GRAPHICS_TYPE(DescriptorSetLayout);
		FLAME_GRAPHICS_TYPE(DescriptorSet);
		FLAME_GRAPHICS_TYPE(Sampler);
		FLAME_GRAPHICS_TYPE(Renderpass);
		FLAME_GRAPHICS_TYPE(Framebuffer);
		FLAME_GRAPHICS_TYPE(PipelineLayout);
		FLAME_GRAPHICS_TYPE(GraphicsPipeline);
		FLAME_GRAPHICS_TYPE(ComputePipeline);
		FLAME_GRAPHICS_TYPE(Swapchain);
		FLAME_GRAPHICS_TYPE(Window);
		FLAME_GRAPHICS_TYPE(Canvas);

		FLAME_GRAPHICS_TYPE(ImageAtlas);
		FLAME_GRAPHICS_TYPE(FontAtlas);
		FLAME_GRAPHICS_TYPE(Material);

		FLAME_GRAPHICS_TYPE(Bone);
		FLAME_GRAPHICS_TYPE(Armature);
		FLAME_GRAPHICS_TYPE(Mesh);
		FLAME_GRAPHICS_TYPE(Channel);
		FLAME_GRAPHICS_TYPE(Animation);

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
			Format_A2R10G10B10_UNORM,
			Format_A2R10G10B10_SNORM,
			Format_R16G16B16A16_UNORM,
			Format_R16G16B16A16_SFLOAT,
			Format_R32G32B32A32_SFLOAT,
			Format_R32G32B32A32_INT,
			Format_B10G11R11_UFLOAT,
			Format_BC1_RGB_UNORM,
			Format_BC1_RGB_SRGB,
			Format_BC1_RGBA_UNORM,
			Format_BC1_RGBA_SRGB,
			Format_BC2_UNORM,
			Format_BC2_SRGB,
			Format_BC3_UNORM,
			Format_BC3_SRGB,
			Format_BC4_UNORM,
			Format_BC4_SNORM,
			Format_BC5_UNORM,
			Format_BC5_SNORM,
			Format_BC6H_UFLOAT,
			Format_BC6H_SFLOAT,
			Format_BC7_UNORM,
			Format_BC7_SRGB,
			Format_RGBA_ETC2,

			Format_Depth16,
			Format_Depth32,
			Format_Stencil8,
			Format_Depth24Stencil8,

			Format_Color_Begin = Format_R8_UNORM,
			Format_Color_End = Format_RGBA_ETC2,
			Format_ColorOneChannel_Begin = Format_R8_UNORM,
			Format_ColorOneChannel_End = Format_R32_SFLOAT,
			Format_Stencil_Begin = Format_Stencil8,
			Format_Stencil_End = Format_Stencil8,
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

			MemoryProperty_Max = 0xffffffff
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

			SampleCount_Max = 0xffffffff
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

			BufferUsage_Max = 0xffffffff
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

			ImageUsage_Max = 0xffffffff
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

		struct ImageSub
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

		enum Filter
		{
			FilterNearest,
			FilterLinear
		};

		enum AddressMode
		{
			AddressClampToEdge,
			AddressClampToBorder,
			AddressRepeat
		};

		enum BorderColor
		{
			BorderColorBlack,
			BorderColorWhite,
			BorderColorTransparentBlack,
			BorderColorTransparentWhite,
			BorderColorCustom
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

		enum PipelineStageFlags
		{
			PipelineStageTop = 1 << 0,
			PipelineStageDrawIndirect = 1 << 1,
			PipelineStageVertexInput = 1 << 2,
			PipelineStageVertShader = 1 << 3,
			PipelineStageTescShader = 1 << 4,
			PipelineStageTeseShader = 1 << 5,
			PipelineStageGeomShader = 1 << 6,
			PipelineStageFragShader = 1 << 7,
			PipelineStageEarlyFragTestShader = 1 << 8,
			PipelineStageLateFragTestShader = 1 << 9,
			PipelineStageColorAttachmentOutput = 1 << 10,
			PipelineStageCompShader = 1 << 11,
			PipelineStageTransfer = 1 << 12,
			PipelineStageBottom = 1 << 13,
			PipelineStageHost = 1 << 14,
			PipelineStageAllGraphics = 1 << 15,
			PipelineStageAllCommand = 1 << 16
		};

		inline PipelineStageFlags operator| (PipelineStageFlags a, PipelineStageFlags b) { return (PipelineStageFlags)((int)a | (int)b); }

		enum AttachmentLoadOp
		{
			AttachmentLoadLoad,
			AttachmentLoadClear,
			AttachmentLoadDontCare
		};

		enum AttachmentStoreOp
		{
			AttachmentStoreStore,
			AttachmentStoreDontCare
		};

		enum QueueFamily
		{
			QueueGraphics,
			QueueTransfer
		};

		enum DescriptorType
		{
			DescriptorNone,
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

		enum StencilOp
		{
			StencilOpKeep,
			StencilOpZero,
			StencilOpReplace,
			StencilOpIncrementAndClamp,
			StencilOpDecrementAndClamp,
			StencilOpInvert,
			StencilOpIncrementAndWrap,
			StencilOpDecrementAndWrap
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
			ShaderStageTask = 1 << 6,
			ShaderStageMesh = 1 << 7,
			ShaderVi		= 1 << 8,
			ShaderDsl		= 1 << 9,
			ShaderPll		= 1 << 10,
			ShaderStageAll  = ShaderStageVert | ShaderStageTesc | ShaderStageTese | ShaderStageGeom | ShaderStageFrag | 
								ShaderStageComp | ShaderStageTask | ShaderStageMesh,

			ShaderStage_Max = 0xffffffff
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

		enum BlendOp
		{
			BlendOpAdd,
			BlendOpSubtract,
			BlendOpReverseSubtract,
			BlendOpMin,
			BlendOpMax
		};

		enum ColorComponentFlags
		{
			ColorComponentNone = 0,
			ColorComponentR = 1 << 0,
			ColorComponentG = 1 << 1,
			ColorComponentB = 1 << 2,
			ColorComponentA = 1 << 3,
			ColorComponentAll = ColorComponentR | ColorComponentG | ColorComponentB | ColorComponentA
		};

		inline ColorComponentFlags operator| (ColorComponentFlags a, ColorComponentFlags b) { return (ColorComponentFlags)((int)a | (int)b); }

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

		constexpr inline AccessFlags u2a(BufferUsageFlags u)
		{
			if (u & BufferUsageVertex)
				return AccessVertexAttributeRead;
			if (u & BufferUsageIndex)
				return AccessIndexRead;
			if (u & BufferUsageIndirect)
				return AccessIndirectCommandRead;
			if (u & BufferUsageUniform)
				return AccessShaderRead;
			if (u & BufferUsageStorage)
				return AccessShaderRead | AccessShaderWrite;
			return AccessNone;
		}

		constexpr inline PipelineStageFlags u2s(BufferUsageFlags u)
		{
			if (u & BufferUsageVertex)
				return PipelineStageVertexInput;
			if (u & BufferUsageIndex)
				return PipelineStageVertexInput;
			if (u & BufferUsageIndirect)
				return PipelineStageDrawIndirect;
			return PipelineStageAllCommand;
		}

		enum class RenderQueue
		{
			Opaque,
			AlphaTest,
			Transparent,
			Count
		};

		struct Glyph
		{
			ushort code = 0;
			ivec2 off = ivec2(0);
			uvec2 size = uvec2(0);
			vec4 uv = vec4(0.f);
			int advance = 0;
		};

		struct Point
		{
			vec3 pos;
			cvec4 col;
		};

		struct Line
		{
			Point a;
			Point b;
		};

		struct Particle
		{
			vec3 pos;
			vec3 xext;
			vec3 yext;
			vec4 uvs;
			vec4 col;
		};

		struct ImageDesc
		{
			ImageViewPtr view = nullptr;
			vec4 uvs;
			vec4 border_uvs;
		};

		struct SparseSlots
		{
			uint capacity;
			std::deque<uint> free_slots;

			void init(uint _capacity)
			{
				capacity = _capacity;
				free_slots.resize(capacity);
				std::iota(free_slots.begin(), free_slots.end(), 0);
			}

			inline int get_free_item()
			{
				if (free_slots.empty())
					return -1;
				auto ret = free_slots.front();
				free_slots.pop_front();
				return ret;
			}

			inline void release_item(uint id)
			{
				free_slots.push_back(id);
			}
		};

		struct SparseRanges
		{
			uint capacity;
			std::vector<std::pair<uint, uint>> free_spaces;

			void init(uint _capacity)
			{
				capacity = _capacity;
				free_spaces.clear();
				free_spaces.emplace_back(0, capacity);
			}

			inline int get_free_space(uint size)
			{
				for (uint i = 0; i < free_spaces.size(); ++i)
				{
					auto& fs = free_spaces[i];
					if (fs.second >= size)
					{
						auto ret = fs.first;
						fs.first += size;
						fs.second -= size;
						if (fs.second == 0)
							free_spaces.erase(free_spaces.begin() + i);
						return ret;
					}
				}
				return -1;
			}

			inline void release_space(uint off, uint size)
			{
				for (uint i = 0; i < free_spaces.size(); ++i)
				{
					auto& fs = free_spaces[i];
					if (fs.first + fs.second == off)
					{
						fs.second += size;
						if (i + 1 < free_spaces.size() && fs.first + fs.second == free_spaces[i + 1].first)
						{
							fs.second += free_spaces[i + 1].second;
							free_spaces.erase(free_spaces.begin() + i + 1);
						}
						return;
					}
					if (off + size == fs.first)
					{
						fs.first -= size;
						fs.second += size;
						if (i > 0 && free_spaces[i - 1].first + free_spaces[i - 1].second == fs.first)
						{
							free_spaces[i - 1].second += fs.second;
							free_spaces.erase(free_spaces.begin() + i);
						}
						return;
					}
					if (off < fs.first)
					{
						free_spaces.insert(free_spaces.begin() + i, { off, size });
						return;
					}
				}
				free_spaces.emplace_back(off, size);
			}
		};
	}
}
