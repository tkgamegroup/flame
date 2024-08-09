#pragma once

#include "../foundation/system.h"
#include "graphics.h"

#if USE_D3D12
#define NOMINMAX
#include <d3d12.h>
#include <dxgi1_6.h>
#elif USE_VULKAN
#include <vulkan/vulkan.h>
#undef INFINITE
extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel;
extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel;
typedef void(VKAPI_PTR* PFN_vkCmdDrawMeshTasksEXT)(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
extern PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasks;
extern PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectName;
#endif

namespace flame
{
	namespace graphics
	{

#if USE_D3D12
		inline void check_dx_result(HRESULT res)
		{
			if (FAILED(res))
			{
				printf("=======================\n");
				printf("graphics error: %d\n", res);
				printf("STACK TRACE:\n");
				auto frames = get_call_frames();
				auto frames_infos = get_call_frames_infos(frames);
				for (auto& info : frames_infos)
					printf("%s() %s:%d\n", info.function.c_str(), info.file.c_str(), info.lineno);
				printf("=======================\n");
				assert(0);
			}
		}

		template<typename T>
		uint to_dx_flags(uint);

		inline DXGI_FORMAT to_dx(Format f)
		{
			switch (f)
			{
			case Format_R32_SFLOAT:
				return DXGI_FORMAT_R32_FLOAT;
			case Format_R32G32_SFLOAT:
				return DXGI_FORMAT_R32G32_FLOAT;
			case Format_R32G32B32_SFLOAT:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case Format_R32G32B32A32_SFLOAT:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case Format_R8G8B8A8_UNORM:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case Format_B8G8R8A8_UNORM:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			default:
				assert(0);
			}
		}

		template<>
		inline uint to_dx_flags<BufferUsageFlags>(uint u)
		{
			uint ret = 0;
			if (u & BufferUsageTransferSrc)
				ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
			if (u & BufferUsageTransferDst)
				ret |= D3D12_RESOURCE_STATE_COPY_DEST;
			if (u & BufferUsageUniform)
				ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			if (u & BufferUsageStorage)
				ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			if (u & BufferUsageVertex)
				ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			if (u & BufferUsageIndex)
				ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
			if (u & BufferUsageIndirect)
				ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
			return ret;
		}

		inline D3D12_RESOURCE_STATES to_dx(uint u, Format fmt = Format_Undefined, SampleCount sc = SampleCount_1)
		{
			if (u & ImageUsageTransferSrc)
				return D3D12_RESOURCE_STATE_GENERIC_READ;
			if (u & ImageUsageTransferDst)
				return D3D12_RESOURCE_STATE_COPY_DEST;
			if (u & ImageUsageSampled)
				return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			if (u & ImageUsageStorage)
				return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			if (u & ImageUsageAttachment)
			{
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
					return D3D12_RESOURCE_STATE_STREAM_OUT;
				else
					return (D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ);
				if (sc != SampleCount_1 && !(fmt >= Format_Depth_Begin && fmt <= Format_Depth_End))
					return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
			}
			return D3D12_RESOURCE_STATE_COMMON;
		}

		inline D3D12_RESOURCE_STATES to_dx(ImageLayout l, Format fmt)
		{
			switch (l)
			{
			case ImageLayoutAttachment:
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
					return D3D12_RESOURCE_STATE_RENDER_TARGET;
				else
					return D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ; // TODO: is it true?
			case ImageLayoutShaderReadOnly:
				return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE/*D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE*/; // TOOD: wtf?
				//case ImageLayoutShaderStorage:
				//	return VK_IMAGE_LAYOUT_GENERAL; // TODO
			case ImageLayoutTransferSrc:
				return D3D12_RESOURCE_STATE_COPY_SOURCE; // TODO: is it true?
			case ImageLayoutTransferDst:
				return D3D12_RESOURCE_STATE_COPY_DEST; // TODO: is it true?
			case ImageLayoutPresent:
				return D3D12_RESOURCE_STATE_PRESENT;
			case ImageLayoutGeneral:
				return D3D12_RESOURCE_STATE_COMMON; // TOOD: is it true?
			}
			return D3D12_RESOURCE_STATE_COMMON; // TOOD: is it true?
		}

		inline D3D12_FILL_MODE to_dx(PolygonMode m)
		{
			switch (m)
			{
			case PolygonModeFill:
				return D3D12_FILL_MODE_SOLID;
			case PolygonModeLine:
				return D3D12_FILL_MODE_WIREFRAME;
			case PolygonModePoint:
				return D3D12_FILL_MODE_SOLID; // TODO: not supported?
			}
		}

		inline D3D12_COMPARISON_FUNC to_dx(CompareOp o)
		{
			switch (o)
			{
			case CompareOpLess:
				return D3D12_COMPARISON_FUNC_LESS;
			case CompareOpLessOrEqual:
				return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case CompareOpGreater:
				return D3D12_COMPARISON_FUNC_GREATER;
			case CompareOpGreaterOrEqual:
				return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case CompareOpEqual:
				return D3D12_COMPARISON_FUNC_EQUAL;
			case CompareOpNotEqual:
				return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case CompareOpAlways:
				return D3D12_COMPARISON_FUNC_ALWAYS;
			}
		}

		inline D3D12_STENCIL_OP to_dx(StencilOp o)
		{
			switch (o)
			{
			case StencilOpKeep:
				return D3D12_STENCIL_OP_KEEP;
			case StencilOpZero:
				return D3D12_STENCIL_OP_ZERO;
			case StencilOpReplace:
				return D3D12_STENCIL_OP_REPLACE;
			case StencilOpIncrementAndClamp:
				return D3D12_STENCIL_OP_INCR_SAT;
			case StencilOpDecrementAndClamp:
				return D3D12_STENCIL_OP_DECR_SAT;
			case StencilOpInvert:
				return D3D12_STENCIL_OP_INVERT;
			case StencilOpIncrementAndWrap:
				return D3D12_STENCIL_OP_INCR;
			case StencilOpDecrementAndWrap:
				return D3D12_STENCIL_OP_DECR;
			}
		}

		inline D3D12_CULL_MODE to_dx(CullMode m)
		{
			switch (m)
			{
			case CullModeNone:
				return D3D12_CULL_MODE_NONE;
			case CullModeFront:
				return D3D12_CULL_MODE_FRONT;
			case CullModeBack:
				return D3D12_CULL_MODE_BACK;
			case CullModeFrontAndback:
				return D3D12_CULL_MODE_FRONT; // TODO: not supported?
			}
		}

		inline D3D12_PRIMITIVE_TOPOLOGY_TYPE to_dx(PrimitiveTopology t)
		{
			switch (t)
			{
			case PrimitiveTopologyPointList:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case PrimitiveTopologyLineList:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
				//case PrimitiveTopologyLineStrip: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveTopologyTriangleList:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				//case PrimitiveTopologyTriangleStrip: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				//case PrimitiveTopologyTriangleFan: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
				//case PrimitiveTopologyLineListWithAdjacency: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
				//case PrimitiveTopologyLineStripWithAdjacency: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
				//case PrimitiveTopologyTriangleListWithAdjacency: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
				//case PrimitiveTopologyTriangleStripWithAdjacency: // TODO: unsupported
				//	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
			case PrimitiveTopologyPatchList:
				return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			}
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
		}
#elif USE_VULKAN
		inline void check_vk_result(VkResult res)
		{
			if (res != VK_SUCCESS)
			{
				printf("=======================\n");
				printf("graphics error: %d\n", res);
				printf("STACK TRACE:\n");
				auto frames = get_call_frames();
				auto frames_infos = get_call_frames_infos(frames);
				for (auto& info : frames_infos)
					printf("%s() %s:%d\n", info.function.c_str(), info.file.c_str(), info.lineno);
				printf("=======================\n");
				assert(0);
			}
		}

		template<typename T>
		VkFlags to_vk_flags(uint);

		inline VkFormat to_vk(Format f)
		{
			switch (f)
			{
			case Format_R8_UNORM:
				return VK_FORMAT_R8_UNORM;
			case Format_R16_UNORM:
				return VK_FORMAT_R16_UNORM;
			case Format_R16_SFLOAT:
				return VK_FORMAT_R16_SFLOAT;
			case Format_R32_SFLOAT:
				return VK_FORMAT_R32_SFLOAT;
			case Format_R32G32_SFLOAT:
				return VK_FORMAT_R32G32_SFLOAT;
			case Format_R32G32B32_SFLOAT:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case Format_R8G8B8A8_UNORM:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case Format_R8G8B8A8_SRGB:
				return VK_FORMAT_R8G8B8A8_SRGB;
			case Format_B8G8R8A8_UNORM:
				return VK_FORMAT_B8G8R8A8_UNORM;
			case Format_B8G8R8A8_SRGB:
				return VK_FORMAT_B8G8R8A8_SRGB;
			case Format_A2R10G10B10_UNORM:
				return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case Format_A2R10G10B10_SNORM:
				return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
			case Format_R16G16B16A16_UNORM:
				return VK_FORMAT_R16G16B16A16_UNORM;
			case Format_R16G16B16A16_SFLOAT:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case Format_R32G32B32A32_SFLOAT:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case Format_R32G32B32A32_INT:
				return VK_FORMAT_R32G32B32A32_SINT;
			case Format_B10G11R11_UFLOAT:
				return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			case Format_BC1_RGB_UNORM:
				return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
			case Format_BC1_RGB_SRGB:
				return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
			case Format_BC1_RGBA_UNORM:
				return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case Format_BC1_RGBA_SRGB:
				return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
			case Format_BC2_UNORM:
				return VK_FORMAT_BC2_UNORM_BLOCK;
			case Format_BC2_SRGB:
				return VK_FORMAT_BC2_SRGB_BLOCK;
			case Format_BC3_UNORM:
				return VK_FORMAT_BC3_UNORM_BLOCK;
			case Format_BC3_SRGB:
				return VK_FORMAT_BC3_SRGB_BLOCK;
			case Format_BC4_UNORM:
				return VK_FORMAT_BC4_UNORM_BLOCK;
			case Format_BC4_SNORM:
				return VK_FORMAT_BC4_SNORM_BLOCK;
			case Format_BC5_UNORM:
				return VK_FORMAT_BC5_UNORM_BLOCK;
			case Format_BC5_SNORM:
				return VK_FORMAT_BC5_SNORM_BLOCK;
			case Format_BC6H_UFLOAT:
				return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			case Format_BC6H_SFLOAT:
				return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			case Format_BC7_UNORM:
				return VK_FORMAT_BC7_UNORM_BLOCK;
			case Format_BC7_SRGB:
				return VK_FORMAT_BC7_SRGB_BLOCK;
			case Format_RGBA_ETC2:
				return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
			case Format_Stencil8:
				return VK_FORMAT_S8_UINT;
			case Format_Depth16:
				return VK_FORMAT_D16_UNORM;
			case Format_Depth32:
				return VK_FORMAT_D32_SFLOAT;
			case Format_Depth24Stencil8:
				return VK_FORMAT_D24_UNORM_S8_UINT;

			default:
				assert(0);
			}
		}

		inline Format from_backend(VkFormat f)
		{
			switch (f)
			{
			case VK_FORMAT_B8G8R8A8_UNORM:
				return Format_B8G8R8A8_UNORM;
			case VK_FORMAT_B8G8R8A8_SRGB:
				return Format_B8G8R8A8_SRGB;
			default:
				assert(0);
			}
		}

		template<>
		inline VkFlags to_vk_flags<MemoryPropertyFlags>(uint p)
		{
			VkMemoryPropertyFlags ret = 0;
			if (p & MemoryPropertyDevice)
				ret |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (p & MemoryPropertyHost)
				ret |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			if (p & MemoryPropertyCoherent)
				ret |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			return ret;
		}

		inline VkSampleCountFlagBits to_vk(SampleCount s)
		{
			switch (s)
			{
			case SampleCount_1:
				return VK_SAMPLE_COUNT_1_BIT;
			case SampleCount_2:
				return VK_SAMPLE_COUNT_2_BIT;
			case SampleCount_4:
				return VK_SAMPLE_COUNT_4_BIT;
			case SampleCount_8:
				return VK_SAMPLE_COUNT_8_BIT;
			case SampleCount_16:
				return VK_SAMPLE_COUNT_16_BIT;
			case SampleCount_32:
				return VK_SAMPLE_COUNT_32_BIT;
			}
		}

		inline VkShaderStageFlagBits to_vk(ShaderStageFlags t)
		{
			switch (t)
			{
			case ShaderStageVert:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderStageTesc:
				return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			case ShaderStageTese:
				return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			case ShaderStageGeom:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			case ShaderStageFrag:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case ShaderStageComp:
				return VK_SHADER_STAGE_COMPUTE_BIT;
#if defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
			case ShaderStageTask:
				return VK_SHADER_STAGE_TASK_BIT_EXT;
			case ShaderStageMesh:
				return VK_SHADER_STAGE_MESH_BIT_EXT;
#endif
			}
			return (VkShaderStageFlagBits)0;
		}

		template<>
		inline VkFlags to_vk_flags<ShaderStageFlags>(uint t)
		{
			VkShaderStageFlags ret = 0;
			if (t & ShaderStageVert)
				ret |= VK_SHADER_STAGE_VERTEX_BIT;
			if (t & ShaderStageTesc)
				ret |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			if (t & ShaderStageTese)
				ret |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			if (t & ShaderStageGeom)
				ret |= VK_SHADER_STAGE_GEOMETRY_BIT;
			if (t & ShaderStageFrag)
				ret |= VK_SHADER_STAGE_FRAGMENT_BIT;
			if (t & ShaderStageComp)
				ret |= VK_SHADER_STAGE_COMPUTE_BIT;
#if defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
			if (t & ShaderStageTask)
				ret |= VK_SHADER_STAGE_TASK_BIT_EXT;
			if (t & ShaderStageMesh)
				ret |= VK_SHADER_STAGE_MESH_BIT_EXT;
#endif
			return ret;
		}

		inline VkDescriptorType to_vk(DescriptorType t)
		{
			switch (t)
			{
			case DescriptorUniformBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorStorageBuffer:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case DescriptorSampledImage:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case DescriptorStorageImage:
				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			}
		}

		inline VkPipelineBindPoint to_vk(PipelineType t)
		{
			switch (t)
			{
			case PipelineNone:
				return VkPipelineBindPoint(-1);
			case PipelineGraphics:
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			case PipelineCompute:
				return VK_PIPELINE_BIND_POINT_COMPUTE;
			}
		}

		template<>
		inline VkFlags to_vk_flags<BufferUsageFlags>(uint u)
		{
			VkBufferUsageFlags ret = 0;
			if (u & BufferUsageTransferSrc)
				ret |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			if (u & BufferUsageTransferDst)
				ret |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (u & BufferUsageUniform)
				ret |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			if (u & BufferUsageStorage)
				ret |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			if (u & BufferUsageVertex)
				ret |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			if (u & BufferUsageIndex)
				ret |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			if (u & BufferUsageIndirect)
				ret |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
			return ret;
		}

		inline VkImageUsageFlags to_vk_flags(ImageUsageFlags u, Format fmt = Format_Undefined, SampleCount sc = SampleCount_1)
		{
			VkImageUsageFlags ret = 0;
			if (u & ImageUsageTransferSrc)
				ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (u & ImageUsageTransferDst)
				ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (u & ImageUsageSampled)
				ret |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (u & ImageUsageStorage)
				ret |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (u & ImageUsageAttachment)
			{
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
					ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				else
					ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				if (sc != SampleCount_1 && !(fmt >= Format_Depth_Begin && fmt <= Format_Depth_End))
				{
					ret |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
					ret = ret & ~VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
					ret = ret & ~VK_IMAGE_USAGE_TRANSFER_DST_BIT;
					ret = ret & ~VK_IMAGE_USAGE_SAMPLED_BIT;
					ret = ret & ~VK_IMAGE_USAGE_STORAGE_BIT;
				}
			}
			return ret;
		}

		inline VkImageLayout to_vk(ImageLayout l, Format fmt)
		{
			switch (l)
			{
			case ImageLayoutAttachment:
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
					return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				else
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case ImageLayoutShaderReadOnly:
				return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case ImageLayoutShaderStorage:
				return VK_IMAGE_LAYOUT_GENERAL;
			case ImageLayoutTransferSrc:
				return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case ImageLayoutTransferDst:
				return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case ImageLayoutPresent:
				return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			case ImageLayoutGeneral:
				return VK_IMAGE_LAYOUT_GENERAL;
			}
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		template<>
		inline VkFlags to_vk_flags<ImageAspectFlags>(uint a)
		{
			VkImageAspectFlags ret = 0;
			if (a & ImageAspectColor)
				ret |= VK_IMAGE_ASPECT_COLOR_BIT;
			if (a & ImageAspectDepth)
				ret |= VK_IMAGE_ASPECT_DEPTH_BIT;
			if (a & ImageAspectStencil)
				ret |= VK_IMAGE_ASPECT_STENCIL_BIT;
			return ret;
		}

		inline VkComponentSwizzle to_vk(Swizzle s)
		{
			switch (s)
			{
			case SwizzleIdentity:
				return VK_COMPONENT_SWIZZLE_IDENTITY;
			case SwizzleZero:
				return VK_COMPONENT_SWIZZLE_ZERO;
			case SwizzleOne:
				return VK_COMPONENT_SWIZZLE_ONE;
			case SwizzleR:
				return VK_COMPONENT_SWIZZLE_R;
			case SwizzleG:
				return VK_COMPONENT_SWIZZLE_G;
			case SwizzleB:
				return VK_COMPONENT_SWIZZLE_B;
			case SwizzleA:
				return VK_COMPONENT_SWIZZLE_A;
			}
		}

		inline VkFilter to_vk(Filter f)
		{
			switch (f)
			{
			case FilterNearest:
				return VK_FILTER_NEAREST;
			case FilterLinear:
				return VK_FILTER_LINEAR;
			}
		}

		inline VkSamplerAddressMode to_vk(AddressMode m)
		{
			switch (m)
			{
			case AddressClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case AddressClampToBorder:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case AddressRepeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
		}

		inline VkBorderColor to_vk(BorderColor c)
		{
			switch (c)
			{
			case BorderColorBlack:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case BorderColorWhite:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case BorderColorTransparentBlack:
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case BorderColorTransparentWhite:
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case BorderColorCustom:
				return VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
			}
		}

		template<>
		inline VkFlags to_vk_flags<AccessFlags>(uint a)
		{
			VkAccessFlags ret = 0;
			if (a & AccessIndirectCommandRead)
				ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
			if (a & AccessIndexRead)
				ret |= VK_ACCESS_INDEX_READ_BIT;
			if (a & AccessVertexAttributeRead)
				ret |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			if (a & AccessUniformRead)
				ret |= VK_ACCESS_UNIFORM_READ_BIT;
			if (a & AccessInputAttachmentRead)
				ret |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			if (a & AccessShaderRead)
				ret |= VK_ACCESS_SHADER_READ_BIT;
			if (a & AccessShaderWrite)
				ret |= VK_ACCESS_SHADER_WRITE_BIT;
			if (a & AccessColorAttachmentRead)
				ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			if (a & AccessColorAttachmentWrite)
				ret |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			if (a & AccessDepthAttachmentRead)
				ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			if (a & AccessDepthAttachmentWrite)
				ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			if (a & AccessTransferRead)
				ret |= VK_ACCESS_TRANSFER_READ_BIT;
			if (a & AccessTransferWrite)
				ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
			if (a & AccessHostRead)
				ret |= VK_ACCESS_HOST_READ_BIT;
			if (a & AccessHostWrite)
				ret |= VK_ACCESS_HOST_WRITE_BIT;
			if (a & AccessMemoryRead)
				ret |= VK_ACCESS_MEMORY_READ_BIT;
			if (a & AccessMemoryWrite)
				ret |= VK_ACCESS_MEMORY_WRITE_BIT;
			return ret;
		}

		template<>
		inline VkFlags to_vk_flags<PipelineStageFlags>(uint s)
		{
			VkPipelineStageFlags ret = 0;
			if (s & PipelineStageTop)
				ret |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			if (s & PipelineStageDrawIndirect)
				ret |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			if (s & PipelineStageVertexInput)
				ret |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			if (s & PipelineStageVertShader)
				ret |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			if (s & PipelineStageTescShader)
				ret |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
			if (s & PipelineStageTeseShader)
				ret |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
			if (s & PipelineStageGeomShader)
				ret |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
			if (s & PipelineStageFragShader)
				ret |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			if (s & PipelineStageEarlyFragTestShader)
				ret |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			if (s & PipelineStageLateFragTestShader)
				ret |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			if (s & PipelineStageColorAttachmentOutput)
				ret |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			if (s & PipelineStageCompShader)
				ret |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			if (s & PipelineStageTransfer)
				ret |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			if (s & PipelineStageBottom)
				ret |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			if (s & PipelineStageHost)
				ret |= VK_PIPELINE_STAGE_HOST_BIT;
			if (s & PipelineStageAllGraphics)
				ret |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			if (s & PipelineStageAllCommand)
				ret |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			return ret;
		}

		inline VkAttachmentLoadOp to_vk(AttachmentLoadOp op)
		{
			switch (op)
			{
			case AttachmentLoadLoad:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
			case AttachmentLoadClear:
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case AttachmentLoadDontCare:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}
		}

		inline VkAttachmentStoreOp to_vk(AttachmentStoreOp op)
		{
			switch (op)
			{
			case AttachmentStoreStore:
				return VK_ATTACHMENT_STORE_OP_STORE;
			case AttachmentStoreDontCare:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
		}

		inline VkVertexInputRate to_vk(VertexInputRate r)
		{
			switch (r)
			{
			case VertexInputRateVertex:
				return VK_VERTEX_INPUT_RATE_VERTEX;
			case VertexInputRateInstance:
				return VK_VERTEX_INPUT_RATE_INSTANCE;
			}
		}

		inline VkPrimitiveTopology to_vk(PrimitiveTopology t)
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

		inline VkPolygonMode to_vk(PolygonMode m)
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

		inline VkCompareOp to_vk(CompareOp o)
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

		inline VkStencilOp to_vk(StencilOp o)
		{
			switch (o)
			{
			case StencilOpKeep:
				return VK_STENCIL_OP_KEEP;
			case StencilOpZero:
				return VK_STENCIL_OP_ZERO;
			case StencilOpReplace:
				return VK_STENCIL_OP_REPLACE;
			case StencilOpIncrementAndClamp:
				return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case StencilOpDecrementAndClamp:
				return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case StencilOpInvert:
				return VK_STENCIL_OP_INVERT;
			case StencilOpIncrementAndWrap:
				return VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case StencilOpDecrementAndWrap:
				return VK_STENCIL_OP_DECREMENT_AND_WRAP;
			}
		}

		inline VkCullModeFlagBits to_vk(CullMode m)
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

		inline VkBlendFactor to_vk(BlendFactor f)
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

		inline VkBlendOp to_vk(BlendOp o)
		{
			switch (o)
			{
			case BlendOpAdd:
				return VK_BLEND_OP_ADD;
			case BlendOpSubtract:
				return VK_BLEND_OP_SUBTRACT;
			case BlendOpReverseSubtract:
				return VK_BLEND_OP_REVERSE_SUBTRACT;
			case BlendOpMin:
				return VK_BLEND_OP_MIN;
			case BlendOpMax:
				return VK_BLEND_OP_MAX;
			}
		}

		template<>
		inline VkFlags to_vk_flags<VkColorComponentFlags>(uint c)
		{
			VkColorComponentFlags ret = 0;
			if (c & ColorComponentR)
				ret |= VK_COLOR_COMPONENT_R_BIT;
			if (c & ColorComponentG)
				ret |= VK_COLOR_COMPONENT_G_BIT;
			if (c & ColorComponentB)
				ret |= VK_COLOR_COMPONENT_B_BIT;
			if (c & ColorComponentA)
				ret |= VK_COLOR_COMPONENT_A_BIT;
			return ret;
		}

		inline VkDynamicState to_vk(DynamicState s)
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
#endif
		inline uint format_size(Format f)
		{
			switch (f)
			{
			case Format_R8_UNORM:
				return 1;
			case Format_R32_UINT: case Format_R32_SFLOAT:
				return 4;
			case Format_R32G32_SFLOAT:
				return 8;
			case Format_R32G32B32_SFLOAT:
				return 12;
			case Format_R8G8B8A8_UNORM:
				return 4;
			case Format_R32G32B32A32_SFLOAT:
			case Format_R32G32B32A32_INT:
				return 16;
			default:
				assert(0);
			}
		}

		struct TrackedObject
		{
			std::string type;
			void* obj = nullptr;
			int die_frames = -1;
		};

		extern std::map<void*, TrackedObject> tracked_objects;
		void register_object(void* backend_obj, const std::string& type, void* obj);
		void unregister_object(void* backend_obj);
	}
}
