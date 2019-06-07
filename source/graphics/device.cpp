// MIT License
// 
// Copyright (c) 2019 wjs
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

#include <flame/foundation/foundation.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "pipeline_private.h"
#include "descriptor_private.h"
#include "image_private.h"
#include "commandbuffer_private.h"

#ifdef FLAME_ANDROID
#include <dlfcn.h>
#endif

namespace flame
{
	namespace graphics
	{
		static Device *shared_device;

#if defined(FLAME_VULKAN)
		VkBool32 VKAPI_PTR report_callback(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char*                                 pLayerPrefix,
			const char*                                 pMessage,
			void*                                       pUserData)
		{
			if (messageCode == 1) return VK_FALSE; // THREADING ERROR

			printf("\n=====VK ERROR=====\nERROR NUM:%d\n%s\n==================\n", messageCode, pMessage);

			if (messageCode == 8) return VK_FALSE; // Your computer is not support anisotropy, never mind
			if (messageCode == 10) return VK_FALSE; // Dest AccessMask 0 [None] must have required access bit 4096 [VK_ACCESS_TRANSFER_WRITE_BIT]  when layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, unless the app has previously added a barrier for this transition.
			if (messageCode == 0) return VK_FALSE; // descriptor set bind warmming
			if (messageCode == 2) return VK_FALSE; // Vertex attribute not consumed by vertex shader, never mind
			if (messageCode == 53) return VK_FALSE; // You have gave more clear values, never mind

			if (messageCode == 54 || messageCode == 113246970) return VK_FALSE; // vkCreateDevice: pCreateInfo->pQueueCreateInfos[0].queueFamilyIndex (= 0) is not less than any previously obtained pQueueFamilyPropertyCount from vkGetPhysicalDeviceQueueFamilyProperties (the pQueueFamilyPropertyCount was never obtained)
			if (messageCode == 5) return VK_FALSE; // SPIR-V module not valid: Operand 4 of MemberDecorate requires one of these capabilities: MultiViewport 
			if (messageCode == 13) return VK_FALSE; // Shader expects at least n descriptors but only less provided, never mind
			if (messageCode == 61) return VK_FALSE; // Some descriptor maybe used before any update, never mind
			if (messageCode == 15) return VK_FALSE; // Shader requires VkPhysicalDeviceFeatures::tessellationShader but is not enabled on the device, never mind

			if (messageCode == 52 || messageCode == 440402828) return VK_FALSE; // At Draw time the active render pass is incompatible w/ gfx pipeline

																				// ignore above

			if (messageCode == 6)
				return VK_FALSE;
			if (messageCode == 101) return VK_FALSE; // vkQueuePresentKHR: Presenting image without calling vkGetPhysicalDeviceSurfaceSupportKHR
			if (messageCode == 100) return VK_FALSE; // vkCreateSwapChainKHR(): surface capabilities not retrieved for this physical device
			if (messageCode == 1922 || messageCode == 341838316) return VK_FALSE; // vkCreateSwapChainKHR(): pCreateInfo->surface is not known at this time to be supported for presentation by this device. The vkGetPhysicalDeviceSurfaceSupportKHR() must be called beforehand, and it must return VK_TRUE support with this surface for at least one queue family of this device
			if (messageCode == 24) return VK_FALSE; // Vertex buffers are bound to command buffer but no vertex buffers are attached to this Pipeline State Object.
			if (messageCode == 59) return VK_FALSE; // Descriptor set encountered the following validation error at vkCmdDrawIndexed() time: Descriptor is being used in draw but has not been updated.
			if (messageCode == 63) return VK_FALSE; // vkBeginCommandBuffer(): Secondary Command Buffers may perform better if a valid framebuffer parameter is specified.
			if (messageCode == 14) return VK_FALSE;
			if (messageCode == 12) return VK_FALSE; // Push constant range covering variable starting at offset not accessible from stage
			if (messageCode == 4) return VK_FALSE;  //Pipeline needs renderpass information

			return VK_FALSE;
		}
#endif

		DevicePrivate::DevicePrivate(bool debug)
		{
#if defined(FLAME_VULKAN)

#ifdef FLAME_ANDROID
			auto libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
			if (!libvulkan)
			{
				LOGW("unable to find vulkan");
				return;
			}

			LOGI("start loading vulkan functions");

			vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
			vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan, "vkDestroyInstance"));
			vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dlsym(libvulkan, "vkEnumeratePhysicalDevices"));
			vkGetPhysicalDeviceFeatures =
				reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(dlsym(libvulkan, "vkGetPhysicalDeviceFeatures"));
			vkGetPhysicalDeviceFormatProperties =
				reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceFormatProperties"));
			vkGetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(
				dlsym(libvulkan, "vkGetPhysicalDeviceImageFormatProperties"));
			vkGetPhysicalDeviceProperties =
				reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceProperties"));
			vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(
				dlsym(libvulkan, "vkGetPhysicalDeviceQueueFamilyProperties"));
			vkGetPhysicalDeviceMemoryProperties =
				reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceMemoryProperties"));
			vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkan, "vkGetInstanceProcAddr"));
			vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(libvulkan, "vkGetDeviceProcAddr"));
			vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(dlsym(libvulkan, "vkCreateDevice"));
			vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(dlsym(libvulkan, "vkDestroyDevice"));
			vkEnumerateInstanceExtensionProperties =
				reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(dlsym(libvulkan, "vkEnumerateInstanceExtensionProperties"));
			vkEnumerateDeviceExtensionProperties =
				reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(dlsym(libvulkan, "vkEnumerateDeviceExtensionProperties"));
			vkEnumerateInstanceLayerProperties =
				reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(dlsym(libvulkan, "vkEnumerateInstanceLayerProperties"));
			vkEnumerateDeviceLayerProperties =
				reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(dlsym(libvulkan, "vkEnumerateDeviceLayerProperties"));
			vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(dlsym(libvulkan, "vkGetDeviceQueue"));
			vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(dlsym(libvulkan, "vkQueueSubmit"));
			vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(dlsym(libvulkan, "vkQueueWaitIdle"));
			vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(dlsym(libvulkan, "vkDeviceWaitIdle"));
			vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(dlsym(libvulkan, "vkAllocateMemory"));
			vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(dlsym(libvulkan, "vkFreeMemory"));
			vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(dlsym(libvulkan, "vkMapMemory"));
			vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(dlsym(libvulkan, "vkUnmapMemory"));
			vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(dlsym(libvulkan, "vkFlushMappedMemoryRanges"));
			vkInvalidateMappedMemoryRanges =
				reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(dlsym(libvulkan, "vkInvalidateMappedMemoryRanges"));
			vkGetDeviceMemoryCommitment =
				reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(dlsym(libvulkan, "vkGetDeviceMemoryCommitment"));
			vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(dlsym(libvulkan, "vkBindBufferMemory"));
			vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(dlsym(libvulkan, "vkBindImageMemory"));
			vkGetBufferMemoryRequirements =
				reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(dlsym(libvulkan, "vkGetBufferMemoryRequirements"));
			vkGetImageMemoryRequirements =
				reinterpret_cast<PFN_vkGetImageMemoryRequirements>(dlsym(libvulkan, "vkGetImageMemoryRequirements"));
			vkGetImageSparseMemoryRequirements =
				reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(dlsym(libvulkan, "vkGetImageSparseMemoryRequirements"));
			vkGetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(
				dlsym(libvulkan, "vkGetPhysicalDeviceSparseImageFormatProperties"));
			vkQueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(dlsym(libvulkan, "vkQueueBindSparse"));
			vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(dlsym(libvulkan, "vkCreateFence"));
			vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(dlsym(libvulkan, "vkDestroyFence"));
			vkResetFences = reinterpret_cast<PFN_vkResetFences>(dlsym(libvulkan, "vkResetFences"));
			vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(dlsym(libvulkan, "vkGetFenceStatus"));
			vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(dlsym(libvulkan, "vkWaitForFences"));
			vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(dlsym(libvulkan, "vkCreateSemaphore"));
			vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(dlsym(libvulkan, "vkDestroySemaphore"));
			vkCreateEvent = reinterpret_cast<PFN_vkCreateEvent>(dlsym(libvulkan, "vkCreateEvent"));
			vkDestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(dlsym(libvulkan, "vkDestroyEvent"));
			vkGetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(dlsym(libvulkan, "vkGetEventStatus"));
			vkSetEvent = reinterpret_cast<PFN_vkSetEvent>(dlsym(libvulkan, "vkSetEvent"));
			vkResetEvent = reinterpret_cast<PFN_vkResetEvent>(dlsym(libvulkan, "vkResetEvent"));
			vkCreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(dlsym(libvulkan, "vkCreateQueryPool"));
			vkDestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(dlsym(libvulkan, "vkDestroyQueryPool"));
			vkGetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(dlsym(libvulkan, "vkGetQueryPoolResults"));
			vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(dlsym(libvulkan, "vkCreateBuffer"));
			vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(dlsym(libvulkan, "vkDestroyBuffer"));
			vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(dlsym(libvulkan, "vkCreateBufferView"));
			vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(dlsym(libvulkan, "vkDestroyBufferView"));
			vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(dlsym(libvulkan, "vkCreateImage"));
			vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(dlsym(libvulkan, "vkDestroyImage"));
			vkGetImageSubresourceLayout =
				reinterpret_cast<PFN_vkGetImageSubresourceLayout>(dlsym(libvulkan, "vkGetImageSubresourceLayout"));
			vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(dlsym(libvulkan, "vkCreateImageView"));
			vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(dlsym(libvulkan, "vkDestroyImageView"));
			vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(dlsym(libvulkan, "vkCreateShaderModule"));
			vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(dlsym(libvulkan, "vkDestroyShaderModule"));
			vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(dlsym(libvulkan, "vkCreatePipelineCache"));
			vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(dlsym(libvulkan, "vkDestroyPipelineCache"));
			vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(dlsym(libvulkan, "vkGetPipelineCacheData"));
			vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(dlsym(libvulkan, "vkMergePipelineCaches"));
			vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(dlsym(libvulkan, "vkCreateGraphicsPipelines"));
			vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(dlsym(libvulkan, "vkCreateComputePipelines"));
			vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(dlsym(libvulkan, "vkDestroyPipeline"));
			vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(dlsym(libvulkan, "vkCreatePipelineLayout"));
			vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(dlsym(libvulkan, "vkDestroyPipelineLayout"));
			vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(dlsym(libvulkan, "vkCreateSampler"));
			vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(dlsym(libvulkan, "vkDestroySampler"));
			vkCreateDescriptorSetLayout =
				reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(dlsym(libvulkan, "vkCreateDescriptorSetLayout"));
			vkDestroyDescriptorSetLayout =
				reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(dlsym(libvulkan, "vkDestroyDescriptorSetLayout"));
			vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(dlsym(libvulkan, "vkCreateDescriptorPool"));
			vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(dlsym(libvulkan, "vkDestroyDescriptorPool"));
			vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(dlsym(libvulkan, "vkResetDescriptorPool"));
			vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(dlsym(libvulkan, "vkAllocateDescriptorSets"));
			vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(dlsym(libvulkan, "vkFreeDescriptorSets"));
			vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(dlsym(libvulkan, "vkUpdateDescriptorSets"));
			vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(dlsym(libvulkan, "vkCreateFramebuffer"));
			vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(dlsym(libvulkan, "vkDestroyFramebuffer"));
			vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(dlsym(libvulkan, "vkCreateRenderPass"));
			vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(dlsym(libvulkan, "vkDestroyRenderPass"));
			vkGetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(dlsym(libvulkan, "vkGetRenderAreaGranularity"));
			vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(dlsym(libvulkan, "vkCreateCommandPool"));
			vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(dlsym(libvulkan, "vkDestroyCommandPool"));
			vkResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(dlsym(libvulkan, "vkResetCommandPool"));
			vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(dlsym(libvulkan, "vkAllocateCommandBuffers"));
			vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(dlsym(libvulkan, "vkFreeCommandBuffers"));
			vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(dlsym(libvulkan, "vkBeginCommandBuffer"));
			vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(dlsym(libvulkan, "vkEndCommandBuffer"));
			vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(dlsym(libvulkan, "vkResetCommandBuffer"));
			vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(dlsym(libvulkan, "vkCmdBindPipeline"));
			vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(dlsym(libvulkan, "vkCmdSetViewport"));
			vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(dlsym(libvulkan, "vkCmdSetScissor"));
			vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(dlsym(libvulkan, "vkCmdSetLineWidth"));
			vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(dlsym(libvulkan, "vkCmdSetDepthBias"));
			vkCmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(dlsym(libvulkan, "vkCmdSetBlendConstants"));
			vkCmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(dlsym(libvulkan, "vkCmdSetDepthBounds"));
			vkCmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(dlsym(libvulkan, "vkCmdSetStencilCompareMask"));
			vkCmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(dlsym(libvulkan, "vkCmdSetStencilWriteMask"));
			vkCmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(dlsym(libvulkan, "vkCmdSetStencilReference"));
			vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(dlsym(libvulkan, "vkCmdBindDescriptorSets"));
			vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(dlsym(libvulkan, "vkCmdBindIndexBuffer"));
			vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(dlsym(libvulkan, "vkCmdBindVertexBuffers"));
			vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(dlsym(libvulkan, "vkCmdDraw"));
			vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(dlsym(libvulkan, "vkCmdDrawIndexed"));
			vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(dlsym(libvulkan, "vkCmdDrawIndirect"));
			vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(dlsym(libvulkan, "vkCmdDrawIndexedIndirect"));
			vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(dlsym(libvulkan, "vkCmdDispatch"));
			vkCmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(dlsym(libvulkan, "vkCmdDispatchIndirect"));
			vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(dlsym(libvulkan, "vkCmdCopyBuffer"));
			vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(dlsym(libvulkan, "vkCmdCopyImage"));
			vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(dlsym(libvulkan, "vkCmdBlitImage"));
			vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(dlsym(libvulkan, "vkCmdCopyBufferToImage"));
			vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(dlsym(libvulkan, "vkCmdCopyImageToBuffer"));
			vkCmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(dlsym(libvulkan, "vkCmdUpdateBuffer"));
			vkCmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(dlsym(libvulkan, "vkCmdFillBuffer"));
			vkCmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(dlsym(libvulkan, "vkCmdClearColorImage"));
			vkCmdClearDepthStencilImage =
				reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(dlsym(libvulkan, "vkCmdClearDepthStencilImage"));
			vkCmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(dlsym(libvulkan, "vkCmdClearAttachments"));
			vkCmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(dlsym(libvulkan, "vkCmdResolveImage"));
			vkCmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(dlsym(libvulkan, "vkCmdSetEvent"));
			vkCmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(dlsym(libvulkan, "vkCmdResetEvent"));
			vkCmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(dlsym(libvulkan, "vkCmdWaitEvents"));
			vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(dlsym(libvulkan, "vkCmdPipelineBarrier"));
			vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(dlsym(libvulkan, "vkCmdBeginQuery"));
			vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(dlsym(libvulkan, "vkCmdEndQuery"));
			vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(dlsym(libvulkan, "vkCmdResetQueryPool"));
			vkCmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(dlsym(libvulkan, "vkCmdWriteTimestamp"));
			vkCmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(dlsym(libvulkan, "vkCmdCopyQueryPoolResults"));
			vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(dlsym(libvulkan, "vkCmdPushConstants"));
			vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(dlsym(libvulkan, "vkCmdBeginRenderPass"));
			vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(dlsym(libvulkan, "vkCmdNextSubpass"));
			vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(dlsym(libvulkan, "vkCmdEndRenderPass"));
			vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(dlsym(libvulkan, "vkCmdExecuteCommands"));
			vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(dlsym(libvulkan, "vkDestroySurfaceKHR"));
			vkGetPhysicalDeviceSurfaceSupportKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceSupportKHR"));
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
			vkGetPhysicalDeviceSurfaceFormatsKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
			vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
			vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(dlsym(libvulkan, "vkCreateSwapchainKHR"));
			vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(dlsym(libvulkan, "vkDestroySwapchainKHR"));
			vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(dlsym(libvulkan, "vkGetSwapchainImagesKHR"));
			vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(dlsym(libvulkan, "vkAcquireNextImageKHR"));
			vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(dlsym(libvulkan, "vkQueuePresentKHR"));
			vkGetPhysicalDeviceDisplayPropertiesKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
			vkGetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
			vkGetDisplayPlaneSupportedDisplaysKHR =
				reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(dlsym(libvulkan, "vkGetDisplayPlaneSupportedDisplaysKHR"));
			vkGetDisplayModePropertiesKHR =
				reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(dlsym(libvulkan, "vkGetDisplayModePropertiesKHR"));
			vkCreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(dlsym(libvulkan, "vkCreateDisplayModeKHR"));
			vkGetDisplayPlaneCapabilitiesKHR =
				reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(dlsym(libvulkan, "vkGetDisplayPlaneCapabilitiesKHR"));
			vkCreateDisplayPlaneSurfaceKHR =
				reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(dlsym(libvulkan, "vkCreateDisplayPlaneSurfaceKHR"));
			vkCreateSharedSwapchainsKHR =
				reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(dlsym(libvulkan, "vkCreateSharedSwapchainsKHR"));

#ifdef VK_USE_PLATFORM_XLIB_KHR
			vkCreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(dlsym(libvulkan, "vkCreateXlibSurfaceKHR"));
			vkGetPhysicalDeviceXlibPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceXlibPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
			vkCreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(dlsym(libvulkan, "vkCreateXcbSurfaceKHR"));
			vkGetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
			vkCreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(dlsym(libvulkan, "vkCreateWaylandSurfaceKHR"));
			vkGetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
			vkCreateMirSurfaceKHR = reinterpret_cast<PFN_vkCreateMirSurfaceKHR>(dlsym(libvulkan, "vkCreateMirSurfaceKHR"));
			vkGetPhysicalDeviceMirPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceMirPresentationSupportKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceMirPresentationSupportKHR"));
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
			vkCreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(dlsym(libvulkan, "vkCreateAndroidSurfaceKHR"));
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
			vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(dlsym(libvulkan, "vkCreateWin32SurfaceKHR"));
			vkGetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(
				dlsym(libvulkan, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif
			vkCreateDebugReportCallbackEXT =
				reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(dlsym(libvulkan, "vkCreateDebugReportCallbackEXT"));
			vkDestroyDebugReportCallbackEXT =
				reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(dlsym(libvulkan, "vkDestroyDebugReportCallbackEXT"));
			vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(dlsym(libvulkan, "vkDebugReportMessageEXT"));

			LOGI("end loading vulkan functions");
#endif

			VkApplicationInfo appInfo;
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = nullptr;
			appInfo.pApplicationName = "";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Flame Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			std::vector<const char*> instLayers;
			if (debug)
				instLayers.push_back("VK_LAYER_LUNARG_standard_validation");

			std::vector<const char*> instExtensions;
			instExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(FLAME_WINDOWS)
			instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(FLAME_ANDROID)
			instExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
			if (debug)
				instExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			VkInstanceCreateInfo instInfo = {};
			instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instInfo.pApplicationInfo = &appInfo;
			instInfo.enabledExtensionCount = instExtensions.size();
			instInfo.ppEnabledExtensionNames = instExtensions.data();
			instInfo.enabledLayerCount = instLayers.size();
			instInfo.ppEnabledLayerNames = instLayers.data();
			vk_chk_res(vkCreateInstance(&instInfo, nullptr, &ins));

			LOGI("vulkan instance created");

			if (debug)
			{
				static auto _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(ins, "vkCreateDebugReportCallbackEXT"));

				VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
				callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
				callbackCreateInfo.pNext = nullptr;
				callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
				callbackCreateInfo.pfnCallback = report_callback;
				callbackCreateInfo.pUserData = nullptr;

				VkDebugReportCallbackEXT callback;
				vk_chk_res(_vkCreateDebugReportCallbackEXT(ins, &callbackCreateInfo, nullptr, &callback));
			}

			uint32_t gpuCount = 0;
			std::vector<VkPhysicalDevice> physical_devices;
			vk_chk_res(vkEnumeratePhysicalDevices(ins, &gpuCount, nullptr));
			LOGI("gpu count:%d", gpuCount);
			physical_devices.resize(gpuCount);
			vk_chk_res(vkEnumeratePhysicalDevices(ins, &gpuCount, physical_devices.data()));
			pd = physical_devices[0];
			if (pd == 0)
				LOGI("physical device not found");

			vkGetPhysicalDeviceProperties(pd, &props);
			unsigned int queue_family_property_count = 0;
			std::vector<VkQueueFamilyProperties> queue_family_properties;
			vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_property_count, nullptr);
			queue_family_properties.resize(queue_family_property_count);
			vkGetPhysicalDeviceQueueFamilyProperties(pd, &queue_family_property_count, queue_family_properties.data());

			float queue_porities[1] = { 0.f };
			std::vector<VkDeviceQueueCreateInfo> queue_infos;

			gq_idx = -1;
			for (auto i = 0; i < queue_family_properties.size(); i++)
			{
				if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					gq_idx = i;
					VkDeviceQueueCreateInfo queue_info = {};
					queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queue_info.queueFamilyIndex = i;
					queue_info.queueCount = 1;
					queue_info.pQueuePriorities = queue_porities;
					queue_infos.push_back(queue_info);
					break;
				}
			}

			tq_idx = -1;
			for (auto i = 0; i < queue_family_properties.size(); i++)
			{
				if (!(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
				{
					tq_idx = i;
					VkDeviceQueueCreateInfo queue_info = {};
					queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queue_info.queueFamilyIndex = i;
					queue_info.queueCount = 1;
					queue_info.pQueuePriorities = queue_porities;
					queue_infos.push_back(queue_info);
					break;
				}
			}

			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(pd, &features);

			vkGetPhysicalDeviceMemoryProperties(pd, &mem_props);

			std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			VkDeviceCreateInfo device_info = {};
			device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			device_info.pQueueCreateInfos = queue_infos.data();
			device_info.queueCreateInfoCount = queue_infos.size();
			device_info.enabledExtensionCount = device_extensions.size();
			device_info.ppEnabledExtensionNames = device_extensions.data();
			device_info.pEnabledFeatures = &features;
			vk_chk_res(vkCreateDevice(pd, &device_info, nullptr, &v));

			LOGI("vulkan device created");

			{
				AttachmentInfo at_info;
				at_info.format = Format_R8G8B8A8_UNORM;
				at_info.clear = true;
				at_info.sample_count = SampleCount_1;
				
				std::vector<int> at_refs = { 0 };
				SubpassInfo sp_info;
				sp_info.color_attachments.count = at_refs.size();
				sp_info.color_attachments.v = at_refs.data();

				std::vector<void*> ats = { &at_info };
				std::vector<void*> sps = { &sp_info };
				RenderpassInfo info;
				info.attachments.count = ats.size();
				info.attachments.v = ats.data();
				info.subpasses.count = sps.size();
				info.subpasses.v = sps.data();

				rp_one_rgba32 = Renderpass::create(this, info);
			}
			{
				GraphicsPipelineInfo info;
				info.shaders.emplace_back(L"2d/fullscreen.vert", "#define UV\n");
				info.shaders.emplace_back(L"2d/tex-trans.frag", "");
				info.renderpass = rp_one_rgba32;
				pl_trans = Pipeline::create(this, info);
			}
			sp_bi_linear = Sampler::create(this, FilterLinear, FilterLinear, false);
			gcp = Commandpool::create(this, gq_idx);
			gq = Queue::create(this, gq_idx);
			if (tq_idx > 0)
			{
				tcp = Commandpool::create(this, tq_idx);
				tq = Queue::create(this, tq_idx);
			}
			else
			{
				tcp = nullptr;
				tq = nullptr;
			}
			dp = Descriptorpool::create(this);

#elif defined(FLAME_D3D12)

			HRESULT res;

			res = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

			auto adapter_idx = 0;
			auto adapter_found = false;

			while (factory->EnumAdapters1(adapter_idx, &adapter) != DXGI_ERROR_NOT_FOUND)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					adapter_idx++;
					continue;
				}

				res = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
				if (SUCCEEDED(res))
				{
					adapter_found = true;
					break;
				}

				adapter_idx++;
			}

			assert(adapter_found);

			res = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&v));
			assert(SUCCEEDED(res));

			gcp = Commandpool::create(this, -1);
			gq = Queue::create(this, -1);
#endif
		}

		DevicePrivate::~DevicePrivate()
		{
		}

		int DevicePrivate::find_memory_type(uint type_filter, int properties /* MemProp */)
		{
#if defined(FLAME_VULKAN)
			for (uint i = 0; i < mem_props.memoryTypeCount; i++)
			{
				if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
					return i;
			}
#elif defined(FLAME_D3D12)

#endif
			return -1;
		}

		bool DevicePrivate::has_feature(Feature f)
		{
#if defined(FLAME_VULKAN)
			switch (f)
			{
			case FeatureTextureCompressionBC:
				return features.textureCompressionBC;
			case FeatureTextureCompressionASTC_LDR:
				return features.textureCompressionASTC_LDR;
			case FeatureTextureCompressionETC2:
				return features.textureCompressionETC2;
			default:
				break;
			}
#elif defined(FLAME_D3D12)

#endif
			return false;
		}

		bool Device::has_feature(Feature f)
		{
			return ((DevicePrivate*)this)->has_feature(f);
		}

		void Device::set_shared()
		{
			shared_device = this;
		}

		Device *Device::create(bool debug)
		{
			return new DevicePrivate(debug);
		}

		Device *Device::get_shared()
		{
			return shared_device;
		}

		void Device::destroy(Device *d)
		{
			delete (DevicePrivate*)d;
		}

		void Device$::initialize$()
		{
			out$o = in$i;
		}

		Device$ bp_device_unused;

	}
}

