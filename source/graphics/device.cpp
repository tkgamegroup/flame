#include "device_private.h"
#include "renderpass_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "command_private.h"

#ifdef FLAME_ANDROID
#include <dlfcn.h>
#endif

namespace flame
{
	namespace graphics
	{
		VkBool32 VKAPI_PTR report_callback(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData)
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

		DevicePrivate::DevicePrivate(bool debug)
		{
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
			instExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
			if (debug)
				instExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			VkInstanceCreateInfo instInfo = {};
			instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instInfo.pApplicationInfo = &appInfo;
			instInfo.enabledExtensionCount = instExtensions.size();
			instInfo.ppEnabledExtensionNames = instExtensions.data();
			instInfo.enabledLayerCount = instLayers.size();
			instInfo.ppEnabledLayerNames = instLayers.data();
			chk_res(vkCreateInstance(&instInfo, nullptr, &vk_instance));

			if (debug)
			{
				static auto _vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugReportCallbackEXT"));

				VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
				callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
				callbackCreateInfo.pNext = nullptr;
				callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
				callbackCreateInfo.pfnCallback = report_callback;
				callbackCreateInfo.pUserData = nullptr;

				VkDebugReportCallbackEXT callback;
				chk_res(_vkCreateDebugReportCallbackEXT(vk_instance, &callbackCreateInfo, nullptr, &callback));
			}

			uint32_t gpu_count = 0;
			std::vector<VkPhysicalDevice> physical_devices;
			chk_res(vkEnumeratePhysicalDevices(vk_instance, &gpu_count, nullptr));
			physical_devices.resize(gpu_count);
			chk_res(vkEnumeratePhysicalDevices(vk_instance, &gpu_count, physical_devices.data()));
			vk_physical_device = physical_devices[0];

			vkGetPhysicalDeviceProperties(vk_physical_device, &vk_props);
			unsigned int queue_family_property_count = 0;
			std::vector<VkQueueFamilyProperties> queue_family_properties;
			vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_property_count, nullptr);
			queue_family_properties.resize(queue_family_property_count);
			vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_property_count, queue_family_properties.data());

			float queue_porities[1] = { 0.f };
			std::vector<VkDeviceQueueCreateInfo> queue_infos;

			graphics_queue_index = -1;
			for (auto i = 0; i < queue_family_properties.size(); i++)
			{
				if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					graphics_queue_index = i;
					VkDeviceQueueCreateInfo queue_info = {};
					queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queue_info.queueFamilyIndex = i;
					queue_info.queueCount = 1;
					queue_info.pQueuePriorities = queue_porities;
					queue_infos.push_back(queue_info);
					break;
				}
			}

			transfer_queue_index = -1;
			for (auto i = 0; i < queue_family_properties.size(); i++)
			{
				if (!(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
					(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
				{
					transfer_queue_index = i;
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
			vkGetPhysicalDeviceFeatures(vk_physical_device, &features);

			vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &vk_mem_props);

			std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			VkDeviceCreateInfo device_info = {};
			device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			device_info.pQueueCreateInfos = queue_infos.data();
			device_info.queueCreateInfoCount = queue_infos.size();
			device_info.enabledExtensionCount = device_extensions.size();
			device_info.ppEnabledExtensionNames = device_extensions.data();
			device_info.pEnabledFeatures = &features;
			chk_res(vkCreateDevice(vk_physical_device, &device_info, nullptr, &vk_device));

			printf("vulkan initialized, gpu count: %d\n", gpu_count);

			descriptor_pool.reset(new DescriptorPoolPrivate(this));
			sampler_nearest.reset(new SamplerPrivate(this, FilterNearest, FilterNearest, false));
			sampler_linear.reset(new SamplerPrivate(this, FilterLinear, FilterLinear, false));
			graphics_command_pool.reset(new CommandPoolPrivate(this, graphics_queue_index));
			graphics_queue.reset(new QueuePrivate(this, graphics_queue_index));
			if (transfer_queue_index > 0)
			{
				transfer_command_pool.reset(new CommandPoolPrivate(this, transfer_queue_index));
				transfer_queue.reset(new QueuePrivate(this, transfer_queue_index));
			}
		}

		DevicePrivate::~DevicePrivate()
		{
		}

		DevicePrivate* _default_device;

		bool DevicePrivate::has_feature(Feature f) const
		{
			switch (f)
			{
			case FeatureTextureCompressionBC:
				return vk_features.textureCompressionBC;
			case FeatureTextureCompressionASTC_LDR:
				return vk_features.textureCompressionASTC_LDR;
			case FeatureTextureCompressionETC2:
				return vk_features.textureCompressionETC2;
			default:
				break;
			}
			return false;
		}

		uint DevicePrivate::find_memory_type(uint type_filter, MemoryPropertyFlags properties)
		{
			auto p = to_backend_flags<MemoryPropertyFlags>(properties);
			for (uint i = 0; i < vk_mem_props.memoryTypeCount; i++)
			{
				if ((type_filter & (1 << i)) && (vk_mem_props.memoryTypes[i].propertyFlags & p) == p)
					return i;
			}
			return -1;
		}

		Device* Device::get_default()
		{
			return _default_device;
		}

		Device* Device::create(bool debug, bool set_to_default)
		{
			auto d = new DevicePrivate(debug);
			if (set_to_default)
				_default_device = d;
			return d;
		}
	}
}

