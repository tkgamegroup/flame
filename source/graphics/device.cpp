#include "device_private.h"
#include "renderpass_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "command_private.h"

namespace flame
{
	namespace graphics
	{
		VkBool32 VKAPI_PTR report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
			size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
		{
			assert(0);
			printf("\n%s\n\n", pMessage);

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
				static auto _vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)(vkGetInstanceProcAddr(vk_instance, "vkCreateDebugReportCallbackEXT"));

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

