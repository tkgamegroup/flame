#include "device_private.h"
#include "renderpass_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "command_private.h"

#define USE_MESH_SHADER 1

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
			static std::regex reg_id(R"(VUID-\w+-\w+-(\d+))");
			static std::regex reg_obj1(R"(Object \d+: handle = 0x(\w+), type \= VK_OBJECT_TYPE_(\w+))");
			static std::regex reg_obj2(R"(Vk(\w+) 0x(\w+)\[\] )");
			auto str = message;
			std::smatch res;
			uint msgid = 0;
			std::vector<std::pair<void*, std::string>> backend_objects;
			if (std::regex_search(str, res, reg_id))
				msgid = s2t<uint>(res[1].str());
			while (std::regex_search(str, res, reg_obj1))
			{
				backend_objects.emplace_back((void*)s2u_hex<uint64>(res[1].str()), res[2].str());
				str = res.suffix();
			}
			while (std::regex_search(str, res, reg_obj2))
			{
				backend_objects.emplace_back((void*)s2u_hex<uint64>(res[2].str()), res[1].str());
				str = res.suffix();
			}

			for (auto& vkobj : backend_objects)
			{
				auto obj = tracked_objects[vkobj.first].obj;
				int cut = 1;
			}

			if (msgid == 2699)
			{
				static std::regex reg_msg(R"(Descriptor in binding \#(\d+) index)");
				int binding = -1;
				if (std::regex_search(str, res, reg_msg))
					binding = s2u_hex<uint>(res[1].str());

				auto ds = (DescriptorSetPtr)tracked_objects[backend_objects[0].first].obj;
				if (ds && binding != -1)
				{
					auto dsl = ds->layout;
					wprintf(L"DSL filename: %s\n", dsl->filename.c_str());
					if (binding < dsl->bindings.size())
						printf("binding %d: %s\n", binding, dsl->bindings[binding].name.c_str());
				}
			}

			return VK_FALSE;
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
				appInfo.apiVersion = VK_API_VERSION_1_3;

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

				ret->vk_props = {};
				ret->vk_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
				auto next_prop = &ret->vk_props.pNext;

				ret->vk_resolve_props = {};
				ret->vk_resolve_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
				*next_prop = &ret->vk_resolve_props;
				next_prop = &ret->vk_resolve_props.pNext;

#if USE_MESH_SHADER
				ret->vk_meshshader_props = {};
				ret->vk_meshshader_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
				*next_prop = &ret->vk_meshshader_props;
				next_prop = &ret->vk_meshshader_props.pNext;
#endif

				vkGetPhysicalDeviceFeatures(physical_device, &ret->vk_features);
				vkGetPhysicalDeviceMemoryProperties(physical_device, &ret->vk_mem_props);
				vkGetPhysicalDeviceProperties2(physical_device, &ret->vk_props); 
				
				printf("gpu: %s\n", ret->vk_props.properties.deviceName);

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

				VkPhysicalDeviceFeatures2 enabled_features = {};
				enabled_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				enabled_features.features = ret->vk_features;
				auto next_feature = &enabled_features.pNext;

#if USE_MESH_SHADER
				VkPhysicalDeviceMeshShaderFeaturesEXT feature_mesh_shader = {};
				feature_mesh_shader.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
				feature_mesh_shader.taskShader = true;
				feature_mesh_shader.meshShader = true;
				*next_feature = &feature_mesh_shader;
				next_feature = &feature_mesh_shader.pNext;
#endif

				VkPhysicalDevice8BitStorageFeaturesKHR feature_8bit_storage = {};
				feature_8bit_storage.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
				feature_8bit_storage.storageBuffer8BitAccess = true;
				*next_feature = &feature_8bit_storage;
				next_feature = &feature_8bit_storage.pNext;

				VkPhysicalDeviceMaintenance4Features feature_maintenance4 = {};
				feature_maintenance4.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
				feature_maintenance4.maintenance4 = true;
				*next_feature = &feature_maintenance4;
				next_feature = &feature_maintenance4.pNext;

				std::vector<const char*> required_device_extensions;
				required_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				required_device_extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
				required_device_extensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
#if USE_MESH_SHADER
				required_device_extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
				required_device_extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
#endif

				VkDeviceCreateInfo device_info = {};
				device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				device_info.pNext = &enabled_features;
				device_info.pQueueCreateInfos = queue_infos.data();
				device_info.queueCreateInfoCount = queue_infos.size();
				device_info.enabledExtensionCount = required_device_extensions.size();
				device_info.ppEnabledExtensionNames = required_device_extensions.data();
				chk_res(vkCreateDevice(physical_device, &device_info, nullptr, &ret->vk_device));
				printf("vulkan: device created\n");

#if USE_MESH_SHADER
				vkCmdDrawMeshTasks = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(ret->vk_device, "vkCmdDrawMeshTasksEXT");
#endif

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
			DevicePtr operator()() override
			{
				return device;
			}
		}Device_current;
		Device::Current& Device::current = Device_current;
	}
}

