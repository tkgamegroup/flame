#pragma once

#include <flame/graphics/graphics.h>

#include <assert.h>

#if defined(FLAME_VULKAN)

#ifdef FLAME_WINDOWS
#define NOMINMAX
#elif FLAME_ANDROID
#define VK_NO_PROTOTYPES 1
#endif
#include <vulkan/vulkan.h>
#ifdef FLAME_WINDOWS
#undef INFINITE
#endif

#ifdef FLAME_ANDROID

// core
extern PFN_vkCreateInstance vkCreateInstance;
extern PFN_vkDestroyInstance vkDestroyInstance;
extern PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
extern PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
extern PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
extern PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
extern PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
extern PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
extern PFN_vkCreateDevice vkCreateDevice;
extern PFN_vkDestroyDevice vkDestroyDevice;
extern PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
extern PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
extern PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
extern PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
extern PFN_vkGetDeviceQueue vkGetDeviceQueue;
extern PFN_vkQueueSubmit vkQueueSubmit;
extern PFN_vkQueueWaitIdle vkQueueWaitIdle;
extern PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
extern PFN_vkAllocateMemory vkAllocateMemory;
extern PFN_vkFreeMemory vkFreeMemory;
extern PFN_vkMapMemory vkMapMemory;
extern PFN_vkUnmapMemory vkUnmapMemory;
extern PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
extern PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
extern PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
extern PFN_vkBindBufferMemory vkBindBufferMemory;
extern PFN_vkBindImageMemory vkBindImageMemory;
extern PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
extern PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
extern PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
extern PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
extern PFN_vkQueueBindSparse vkQueueBindSparse;
extern PFN_vkCreateFence vkCreateFence;
extern PFN_vkDestroyFence vkDestroyFence;
extern PFN_vkResetFences vkResetFences;
extern PFN_vkGetFenceStatus vkGetFenceStatus;
extern PFN_vkWaitForFences vkWaitForFences;
extern PFN_vkCreateSemaphore vkCreateSemaphore;
extern PFN_vkDestroySemaphore vkDestroySemaphore;
extern PFN_vkCreateEvent vkCreateEvent;
extern PFN_vkDestroyEvent vkDestroyEvent;
extern PFN_vkGetEventStatus vkGetEventStatus;
extern PFN_vkSetEvent vkSetEvent;
extern PFN_vkResetEvent vkResetEvent;
extern PFN_vkCreateQueryPool vkCreateQueryPool;
extern PFN_vkDestroyQueryPool vkDestroyQueryPool;
extern PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
extern PFN_vkCreateBuffer vkCreateBuffer;
extern PFN_vkDestroyBuffer vkDestroyBuffer;
extern PFN_vkCreateBufferView vkCreateBufferView;
extern PFN_vkDestroyBufferView vkDestroyBufferView;
extern PFN_vkCreateImage vkCreateImage;
extern PFN_vkDestroyImage vkDestroyImage;
extern PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
extern PFN_vkCreateImageView vkCreateImageView;
extern PFN_vkDestroyImageView vkDestroyImageView;
extern PFN_vkCreateShaderModule vkCreateShaderModule;
extern PFN_vkDestroyShaderModule vkDestroyShaderModule;
extern PFN_vkCreatePipelineCache vkCreatePipelineCache;
extern PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
extern PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
extern PFN_vkMergePipelineCaches vkMergePipelineCaches;
extern PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
extern PFN_vkCreateComputePipelines vkCreateComputePipelines;
extern PFN_vkDestroyPipeline vkDestroyPipeline;
extern PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
extern PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
extern PFN_vkCreateSampler vkCreateSampler;
extern PFN_vkDestroySampler vkDestroySampler;
extern PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
extern PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
extern PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
extern PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
extern PFN_vkResetDescriptorPool vkResetDescriptorPool;
extern PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
extern PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
extern PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
extern PFN_vkCreateFramebuffer vkCreateFramebuffer;
extern PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
extern PFN_vkCreateRenderPass vkCreateRenderPass;
extern PFN_vkDestroyRenderPass vkDestroyRenderPass;
extern PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
extern PFN_vkCreateCommandPool vkCreateCommandPool;
extern PFN_vkDestroyCommandPool vkDestroyCommandPool;
extern PFN_vkResetCommandPool vkResetCommandPool;
extern PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
extern PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
extern PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
extern PFN_vkEndCommandBuffer vkEndCommandBuffer;
extern PFN_vkResetCommandBuffer vkResetCommandBuffer;
extern PFN_vkCmdBindPipeline vkCmdBindPipeline;
extern PFN_vkCmdSetViewport vkCmdSetViewport;
extern PFN_vkCmdSetScissor vkCmdSetScissor;
extern PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
extern PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
extern PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
extern PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
extern PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
extern PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
extern PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
extern PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
extern PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
extern PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
extern PFN_vkCmdDraw vkCmdDraw;
extern PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
extern PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
extern PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
extern PFN_vkCmdDispatch vkCmdDispatch;
extern PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
extern PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
extern PFN_vkCmdCopyImage vkCmdCopyImage;
extern PFN_vkCmdBlitImage vkCmdBlitImage;
extern PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
extern PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
extern PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
extern PFN_vkCmdFillBuffer vkCmdFillBuffer;
extern PFN_vkCmdClearColorImage vkCmdClearColorImage;
extern PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
extern PFN_vkCmdClearAttachments vkCmdClearAttachments;
extern PFN_vkCmdResolveImage vkCmdResolveImage;
extern PFN_vkCmdSetEvent vkCmdSetEvent;
extern PFN_vkCmdResetEvent vkCmdResetEvent;
extern PFN_vkCmdWaitEvents vkCmdWaitEvents;
extern PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
extern PFN_vkCmdBeginQuery vkCmdBeginQuery;
extern PFN_vkCmdEndQuery vkCmdEndQuery;
extern PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
extern PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
extern PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
extern PFN_vkCmdPushConstants vkCmdPushConstants;
extern PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
extern PFN_vkCmdNextSubpass vkCmdNextSubpass;
extern PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
extern PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
// VK_KHR_surface
extern PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
// VK_KHR_swapchain
extern PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
extern PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
extern PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
extern PFN_vkQueuePresentKHR vkQueuePresentKHR;
// VK_KHR_display
extern PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
extern PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
extern PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR;
extern PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
extern PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR;
extern PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR;
extern PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;
// VK_KHR_display_swapchain
extern PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR;
#ifdef VK_USE_PLATFORM_XLIB_KHR
// VK_KHR_xlib_surface
extern PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
// VK_KHR_xcb_surface
extern PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
// VK_KHR_wayland_surface
extern PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
extern PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
// VK_KHR_mir_surface
extern PFN_vkCreateMirSurfaceKHR vkCreateMirSurfaceKHR;
extern PFN_vkGetPhysicalDeviceMirPresentationSupportKHR vkGetPhysicalDeviceMirPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
// VK_KHR_android_surface
extern PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
// VK_KHR_win32_surface
extern PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
extern PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif
// VK_EXT_debug_report
extern PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
extern PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
extern PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;

#endif

#elif defined(FLAME_D3D12)

#include <d3d12.h>
#include <dxgi1_4.h>

#endif

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_VULKAN)

		inline void chk_res(VkResult res)
		{
			assert(res == VK_SUCCESS);
		}

		inline VkFormat to_enum(Format$ f)
		{
			switch (f)
			{
				case Format_R8_UNORM:
					return VK_FORMAT_R8_UNORM;
				case Format_R16_UNORM:
					return VK_FORMAT_R16_UNORM;
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
				case Format_B8G8R8A8_UNORM: case Format_Swapchain_B8G8R8A8_UNORM:
					return VK_FORMAT_B8G8R8A8_UNORM;
				case Format_B8G8R8A8_SRGB: case Format_Swapchain_B8G8R8A8_SRGB:
					return VK_FORMAT_B8G8R8A8_SRGB;
				case Format_R16G16B16A16_UNORM:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case Format_R16G16B16A16_SFLOAT:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case Format_R32G32B32A32_SFLOAT:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case Format_RGBA_BC3:
					return VK_FORMAT_BC3_UNORM_BLOCK;
				case Format_RGBA_ETC2:
					return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
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

		inline Format$ to_enum(VkFormat f, bool is_swapchain)
		{
			switch (f)
			{
				case VK_FORMAT_B8G8R8A8_UNORM:
					return is_swapchain ? Format_Swapchain_B8G8R8A8_UNORM : Format_B8G8R8A8_UNORM;
				case VK_FORMAT_B8G8R8A8_SRGB:
					return is_swapchain ? Format_Swapchain_B8G8R8A8_SRGB : Format_B8G8R8A8_SRGB;
				default:
					assert(0);
			}
		}

		inline std::string cpp_typehash_to_glsl_typename(uint hash)
		{
			switch (hash)
			{
			case cH("int"):
				return "int";
			case cH("Vec(2+int)"):
				return "ivec2";
			case cH("Vec(3+int)"):
				return"ivec3";
			case cH("Vec(4+int)"):
				return"ivec4";
			case cH("float"):
				return"float";
			case cH("Vec(2+float)"):
				return"vec2";
			case cH("Vec(3+float)"):
				return"vec3";
			case cH("Vec(4+float)"):
				return"vec4";
			default:
				assert(0);
			}
		}

		inline std::string format_to_cpp_typename(Format$ f)
		{
			switch (f)
			{
			case Format_R32_UINT:
				return "uint";
			case Format_R8_UNORM: case Format_R32_SFLOAT:
				return "float";
			case Format_R32G32_SFLOAT:
				return "Vec(2+float)";
			case Format_R32G32B32_SFLOAT:
				return "Vec(3+float)";
			case Format_R8G8B8A8_UNORM: case Format_R32G32B32A32_SFLOAT:
				return "Vec(4+float)";
			default:
				assert(0);
			}
		}

		inline std::string format_to_glsl_typename(Format$ f)
		{
			switch (f)
			{
			case Format_R32_UINT:
				return "flat uint";
			case Format_R8_UNORM: case Format_R32_SFLOAT:
				return "float";
			case Format_R32G32_SFLOAT:
				return "vec2";
			case Format_R32G32B32_SFLOAT:
				return "vec3";
			case Format_R8G8B8A8_UNORM: case Format_R32G32B32A32_SFLOAT:
				return "vec4";
			default:
				assert(0);
			}
		}

		inline VkMemoryPropertyFlags to_flags(MemProp$ p)
		{
			VkMemoryPropertyFlags ret = 0;
			if (p & MemPropDevice)
				ret |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (p & MemPropHost)
				ret |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			if (p & MemPropHostCoherent)
				ret |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			return ret;
		}

		inline VkSampleCountFlagBits to_enum(SampleCount$ s)
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

		inline VkShaderStageFlagBits to_enum(ShaderStage$ t)
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
			}
		}

		inline VkShaderStageFlags to_flags(ShaderStage$ t)
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
			return ret;
		}

		inline VkDescriptorType to_enum(DescriptorType$ t)
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

		inline VkPipelineBindPoint to_enum(PipelineType t)
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

		inline VkBufferUsageFlags to_flags(BufferUsage$ u)
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

		inline VkImageUsageFlags to_flags(ImageUsage$ u, Format$ fmt, SampleCount$ sc)
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
				if (sc != SampleCount_1)
					ret |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			}
			return ret;
		}

		inline VkImageLayout to_enum(ImageLayout l, Format$ fmt)
		{
			switch (l)
			{
			case ImageLayoutUndefined:
				return VK_IMAGE_LAYOUT_UNDEFINED;
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
			}
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		inline VkImageAspectFlags to_flags(ImageAspect a)
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

		inline VkImageViewType to_enum(ImageviewType$ t)
		{
			switch (t)
			{
				case Imageview1D:
					return VK_IMAGE_VIEW_TYPE_1D;
				case Imageview2D:
					return VK_IMAGE_VIEW_TYPE_2D;
				case Imageview3D:
					return VK_IMAGE_VIEW_TYPE_3D;
				case ImageviewCube:
					return VK_IMAGE_VIEW_TYPE_CUBE;
				case Imageview1DArray:
					return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				case ImageView2DArray:
					return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				case ImageViewCubeArray:
					return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			}
		}

		inline VkComponentSwizzle to_enum(Swizzle$ s)
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

		inline VkFilter to_enum(Filter f)
		{
			switch (f)
			{
				case FilterNearest:
					return VK_FILTER_NEAREST;
				case FilterLinear:
					return VK_FILTER_LINEAR;
			}
		}

		inline VkVertexInputRate to_enum(VertexInputRate$ r)
		{
			switch (r)
			{
			case VertexInputRateVertex:
				return VK_VERTEX_INPUT_RATE_VERTEX;
			case VertexInputRateInstance:
				return VK_VERTEX_INPUT_RATE_INSTANCE;
			}
		}

		inline VkPrimitiveTopology to_enum(PrimitiveTopology$ t)
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

		inline VkPolygonMode to_enum(PolygonMode m)
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

		inline VkCompareOp to_enum(CompareOp o)
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

		inline VkCullModeFlagBits to_enum(CullMode m)
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

		inline VkBlendFactor to_enum(BlendFactor$ f)
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

		inline VkDynamicState to_enum(DynamicState s)
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

#elif defined(FLAME_D3D12)

		inline DXGI_FORMAT to_enum(Format f)
		{
			switch (f)
			{
			case Format_R8_UNORM:
				return DXGI_FORMAT_R8_UNORM;
			case Format_R16_UNORM:
				return DXGI_FORMAT_R16_UNORM;
			case Format_R32_SFLOAT:
				return DXGI_FORMAT_R32_FLOAT;
			case Format_R32G32_SFLOAT:
				return DXGI_FORMAT_R32G32_FLOAT;
			case Format_R32G32B32_SFLOAT:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case Format_R8G8B8A8_UNORM:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case Format_R8G8B8A8_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			case Format_B8G8R8A8_UNORM: case Format_Swapchain_B8G8R8A8_UNORM:
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			case Format_B8G8R8A8_SRGB: case Format_Swapchain_B8G8R8A8_SRGB:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case Format_R16G16B16A16_UNORM:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
			case Format_R16G16B16A16_SFLOAT:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case Format_R32G32B32A32_SFLOAT:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case Format_RGBA_BC3:
				return DXGI_FORMAT_BC3_UNORM;
			case Format_Depth16:
				return DXGI_FORMAT_D16_UNORM;
			case Format_Depth32:
				return DXGI_FORMAT_D32_FLOAT;
			case Format_Depth24Stencil8:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;

			default:
				assert(0);
			}
		}

		inline D3D12_RESOURCE_STATES to_enum(ImageLayout l)
		{
			switch (l)
			{
			case ImageLayoutAttachment:
				return D3D12_RESOURCE_STATE_RENDER_TARGET;
			case ImageLayoutPresent:
				return D3D12_RESOURCE_STATE_PRESENT;
			}
			return D3D12_RESOURCE_STATE_COMMON;
		}

#endif
	}
}
