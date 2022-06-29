#include "device_private.h"
#include "renderpass_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "command_private.h"

namespace flame
{
	namespace graphics
	{
		DevicePtr device = nullptr;

		VkBool32 VKAPI_PTR report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
			size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
		{
			auto message = std::string(pMessage);
			printf("\n%s\n", message.c_str());

			return VK_FALSE;
		}

		bool DevicePrivate::has_feature(Feature feature) const
		{
			switch (feature)
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

		struct DeviceCreate : Device::Create
		{
			DevicePtr operator()(bool debug) override
			{
				auto ret = new DevicePrivate;

				uint32_t count;
				vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
				std::vector<VkExtensionProperties> instance_extensions(count);
				vkEnumerateInstanceExtensionProperties(nullptr, &count, instance_extensions.data());
				printf("Vulkan Instance Extensions:\n");
				for (auto& extension : instance_extensions)
					printf("  %s\n", extension.extensionName);

				VkApplicationInfo appInfo;
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pNext = nullptr;
				appInfo.pApplicationName = "";
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.pEngineName = "Flame Engine";
				appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;

				std::vector<const char*> required_instance_extensions;
				std::vector<const char*> required_instance_layers;
				required_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				required_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
				if (debug)
				{
					required_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
					required_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
				}
				VkInstanceCreateInfo instInfo = {};
				instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instInfo.pApplicationInfo = &appInfo;
				instInfo.enabledExtensionCount = required_instance_extensions.size();
				instInfo.ppEnabledExtensionNames = required_instance_extensions.empty() ? nullptr : required_instance_extensions.data();
				instInfo.enabledLayerCount = required_instance_layers.size();
				instInfo.ppEnabledLayerNames = required_instance_layers.empty() ? nullptr : required_instance_layers.data();
				chk_res(vkCreateInstance(&instInfo, nullptr, &ret->vk_instance));

				if (debug)
				{
					VkDebugReportCallbackCreateInfoEXT info;
					info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
					info.pNext = nullptr;
					info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
					info.pfnCallback = report_callback;
					info.pUserData = nullptr;

					VkDebugReportCallbackEXT callback;
					chk_res(((PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(ret->vk_instance, "vkCreateDebugReportCallbackEXT"))
						(ret->vk_instance, &info, nullptr, &callback));
				}

				count = 0;
				std::vector<VkPhysicalDevice> physical_devices;
				chk_res(vkEnumeratePhysicalDevices(ret->vk_instance, &count, nullptr));
				physical_devices.resize(count);
				chk_res(vkEnumeratePhysicalDevices(ret->vk_instance, &count, physical_devices.data()));
				auto physical_device = physical_devices[0];
				ret->vk_physical_device = physical_device;

				vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
				std::vector<VkExtensionProperties> device_extensions(count);
				vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, device_extensions.data());
				printf("Vulkan Device Extensions:\n");
				for (auto& extension : device_extensions)
					printf("  %s\n", extension.extensionName);

				vkGetPhysicalDeviceProperties(physical_device, &ret->vk_props);
				printf("gpu: %s\n", ret->vk_props.deviceName);
				vkGetPhysicalDeviceFeatures(physical_device, &ret->vk_features);
				uint queue_family_property_count = 0;
				std::vector<VkQueueFamilyProperties> queue_family_properties;
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, nullptr);
				queue_family_properties.resize(queue_family_property_count);
				vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, queue_family_properties.data());

				float queue_porities[1] = { 0.f };
				std::vector<VkDeviceQueueCreateInfo> queue_infos;

				auto graphics_queue_index = -1;
				auto transfer_queue_index = -1;

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

				//for (auto i = 0; i < queue_family_properties.size(); i++)
				//{
				//	if (!(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
				//		(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
				//	{
				//		transfer_queue_index = i;
				//		VkDeviceQueueCreateInfo queue_info = {};
				//		queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				//		queue_info.queueFamilyIndex = i;
				//		queue_info.queueCount = 1;
				//		queue_info.pQueuePriorities = queue_porities;
				//		queue_infos.push_back(queue_info);
				//		break;
				//	}
				//}

				vkGetPhysicalDeviceMemoryProperties(physical_device, &ret->vk_mem_props);

				std::vector<const char*> required_device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
				VkDeviceCreateInfo device_info = {};
				device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				device_info.pQueueCreateInfos = queue_infos.data();
				device_info.queueCreateInfoCount = queue_infos.size();
				device_info.enabledExtensionCount = required_device_extensions.size();
				device_info.ppEnabledExtensionNames = required_device_extensions.data();
				device_info.pEnabledFeatures = &ret->vk_features;
				chk_res(vkCreateDevice(physical_device, &device_info, nullptr, &ret->vk_device));
				printf("vulkan: device created\n");

				device = ret;

				descriptorset_pool.reset(DescriptorPool::create());
				graphics_command_pool.reset(graphics_queue_index != -1 ? CommandPool::create(graphics_queue_index) : nullptr);
				transfer_command_pool.reset(transfer_queue_index != -1 ? CommandPool::create(transfer_queue_index) : nullptr);
				graphics_queue.reset(graphics_queue_index != -1 ? QueuePrivate::create(graphics_queue_index) : nullptr);
				transfer_queue.reset(transfer_queue_index != -1 ? QueuePrivate::create(transfer_queue_index) : nullptr);
				return ret;
			}
		}Device_create;
		Device::Create& Device::create = Device_create;

		struct DeviceCurrent : Device::Current
		{
			DevicePtr& operator()() override
			{
				return device;
			}
		}Device_current;
		Device::Current& Device::current = Device_current;
	}
}

