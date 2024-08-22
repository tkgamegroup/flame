#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "command_private.h"

namespace flame
{
	namespace graphics
	{
		DevicePtr device = nullptr;


#if USE_D3D12

#elif USE_VULKAN
		VkBool32 VKAPI_PTR report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
			size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
		{
			auto message = std::string(pMessage);

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
				auto& obj = tracked_objects[vkobj.first];
				int cut = 1;
			}

			if (msgid == 2699)
			{
				printf("\n%s\n", message.c_str());

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
			else if (msgid == 727)
			{
				// ignore this
			}
			else
			{
				printf("\n%s\n", message.c_str());
			}

			return VK_FALSE;
		}
#endif

		DevicePrivate::~DevicePrivate()
		{
#if USE_D3D12
			d3d12_device->Release();
			dxgi_factory->Release();
#elif USE_VULKAN
			vkDestroyDevice(vk_device, nullptr);
			vkDestroyInstance(vk_instance, nullptr);
#endif
		}

		bool DevicePrivate::get_config(uint hash, uint & value)
		{
			if (auto it = configs.find(hash); it != configs.end())
			{
				value = it->second;
				return true;
			}
			return false;
		}


#if USE_D3D12

#elif USE_VULKAN
		static void vk_set_object_debug_name(VkDevice vk_device, void* backend_obj, VkObjectType type, const std::string& name)
		{
			VkDebugUtilsObjectNameInfoEXT info = {};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.objectType = type;
			info.objectHandle = (uint64)backend_obj;
			info.pObjectName = name.c_str();
			vkSetDebugUtilsObjectName(vk_device, &info);
		}
#endif

		void DevicePrivate::set_object_debug_name(BufferPtr obj, const std::string& name)
		{
#if USE_D3D12
			obj->d3d12_resource->SetName(s2w(name).c_str());
#elif USE_VULKAN
			vk_set_object_debug_name(vk_device, obj->vk_buffer, VK_OBJECT_TYPE_BUFFER, name);
#endif
		}

		void DevicePrivate::set_object_debug_name(ImagePtr obj, const std::string& name)
		{
#if USE_D3D12
			obj->d3d12_resource->SetName(s2w(name).c_str());
#elif USE_VULKAN
			vk_set_object_debug_name(vk_device, obj->vk_image, VK_OBJECT_TYPE_IMAGE, name);
#endif
		}

#if USE_VULKAN
		uint DevicePrivate::find_memory_type(uint type_filter, MemoryPropertyFlags properties) const
		{
			auto p = to_vk_flags<MemoryPropertyFlags>(properties);
			for (uint i = 0; i < vk_mem_props.memoryTypeCount; i++)
			{
				if ((type_filter & (1 << i)) && (vk_mem_props.memoryTypes[i].propertyFlags & p) == p)
					return i;
			}
			return -1;
		}
#endif

		struct DeviceCreate : Device::Create
		{
			DevicePtr operator()(bool debug, const std::vector<std::pair<uint, uint>>& configs) override
			{
				auto ret = new DevicePrivate;

				for (auto& c : configs)
					ret->configs[c.first] = c.second;

				uint u;
				bool use_mesh_shader = false;
#if USE_VULKAN && defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
				use_mesh_shader = ret->get_config("mesh_shader"_h, u) ? u == 1 : true;
#else
				ret->configs["mesh_shader"_h] = 0;
#endif

#if USE_D3D12
				auto dxgi_flags = 0;
				if (debug)
				{
					ID3D12Debug* debug_controller = nullptr;
					check_dx_result(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
					debug_controller->EnableDebugLayer();
					dxgi_flags |= DXGI_CREATE_FACTORY_DEBUG;
					debug_controller->Release();
				}

				check_dx_result(CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(&ret->dxgi_factory)));

				IDXGIAdapter1* adapter = nullptr;
				IDXGIFactory6* factory6 = nullptr;
				if (SUCCEEDED(ret->dxgi_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
				{
					for (auto i = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); i++)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
							continue;

						if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
							break;
					}
				}
				if (factory6)
					factory6->Release();

				if (!adapter)
				{
					for (UINT i = 0; SUCCEEDED(ret->dxgi_factory->EnumAdapters1(i, &adapter)); i++)
					{
						DXGI_ADAPTER_DESC1 desc;
						adapter->GetDesc1(&desc);

						if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
							continue;

						if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
							break;
					}
				}

				if (adapter)
				{
					DXGI_ADAPTER_DESC1 desc;
					adapter->GetDesc1(&desc);
					wprintf(L"d3d12: use adapter: %s\n", desc.Description);
				}
				else
					printf("d3d12: no adapter found\n");

				check_dx_result(ret->dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
				check_dx_result(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&ret->d3d12_device)));
				ret->d3d12_rtv_size = ret->d3d12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				ret->d3d12_dsv_size = ret->d3d12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
				ret->d3d12_srv_size = ret->d3d12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				ret->d3d12_sp_size = ret->d3d12_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				adapter->Release();
				printf("d3d12: device created\n");
				device = ret;

				descriptorset_pool.reset(DescriptorPool::create());
				graphics_command_pool.reset(CommandPool::create(0));
				transfer_command_pool.reset(CommandPool::create(0));
				graphics_queue.reset(QueuePrivate::create(0));
				transfer_queue.reset(QueuePrivate::create(0));
#elif USE_VULKAN
				uint32_t count;
				vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
				std::vector<VkExtensionProperties> instance_extensions(count);
				vkEnumerateInstanceExtensionProperties(nullptr, &count, instance_extensions.data());
				printf("Vulkan Instance Extensions:\n");
				for (auto& extension : instance_extensions)
					printf("  %s\n", extension.extensionName);

				VkApplicationInfo app_info = {};
				app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				app_info.pApplicationName = "";
				app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				app_info.pEngineName = "Flame Engine";
				app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				app_info.apiVersion = VK_API_VERSION_1_3;

				std::vector<const char*> required_instance_extensions;
				std::vector<const char*> required_instance_layers;
				required_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
				required_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
				required_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				if (debug)
				{
					required_instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
					required_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
				}
				VkInstanceCreateInfo instance_info = {};
				instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instance_info.pApplicationInfo = &app_info;
				instance_info.enabledExtensionCount = required_instance_extensions.size();
				instance_info.ppEnabledExtensionNames = required_instance_extensions.empty() ? nullptr : required_instance_extensions.data();
				instance_info.enabledLayerCount = required_instance_layers.size();
				instance_info.ppEnabledLayerNames = required_instance_layers.empty() ? nullptr : required_instance_layers.data();
				check_vk_result(vkCreateInstance(&instance_info, nullptr, &ret->vk_instance));

				vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(ret->vk_instance, "vkCmdBeginDebugUtilsLabelEXT");
				vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(ret->vk_instance, "vkCmdEndDebugUtilsLabelEXT");
				vkSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(ret->vk_instance, "vkSetDebugUtilsObjectNameEXT");

				if (debug)
				{
					VkDebugReportCallbackCreateInfoEXT info;
					info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
					info.pNext = nullptr;
					info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
					info.pfnCallback = report_callback;
					info.pUserData = nullptr;

					VkDebugReportCallbackEXT callback;
					check_vk_result(((PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(ret->vk_instance, "vkCreateDebugReportCallbackEXT"))
						(ret->vk_instance, &info, nullptr, &callback));
				}

				count = 0;
				std::vector<VkPhysicalDevice> physical_devices;
				check_vk_result(vkEnumeratePhysicalDevices(ret->vk_instance, &count, nullptr));
				physical_devices.resize(count);
				check_vk_result(vkEnumeratePhysicalDevices(ret->vk_instance, &count, physical_devices.data()));
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

#if defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
				if (use_mesh_shader)
				{
					ret->vk_meshshader_props = {};
					ret->vk_meshshader_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
					*next_prop = &ret->vk_meshshader_props;
					next_prop = &ret->vk_meshshader_props.pNext;
				}
#endif

				vkGetPhysicalDeviceFeatures(physical_device, &ret->vk_features);
				vkGetPhysicalDeviceMemoryProperties(physical_device, &ret->vk_mem_props);
				vkGetPhysicalDeviceProperties2(physical_device, &ret->vk_props);

				printf("gpu: %s\n", ret->vk_props.properties.deviceName);
				printf("max pushconst size: %d\n", (int)ret->vk_props.properties.limits.maxPushConstantsSize);

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

#if defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
				if (use_mesh_shader)
				{
					VkPhysicalDeviceMeshShaderFeaturesEXT feature_mesh_shader = {};
					feature_mesh_shader.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
					feature_mesh_shader.taskShader = true;
					feature_mesh_shader.meshShader = true;
					*next_feature = &feature_mesh_shader;
					next_feature = &feature_mesh_shader.pNext;
				}
#endif

				VkPhysicalDevice8BitStorageFeaturesKHR feature_8bit_storage = {};
				feature_8bit_storage.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR;
				feature_8bit_storage.storageBuffer8BitAccess = true;
				*next_feature = &feature_8bit_storage;
				next_feature = &feature_8bit_storage.pNext;

				VkPhysicalDeviceCustomBorderColorFeaturesEXT feature_custom_border_color = {};
				feature_custom_border_color.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT;
				feature_custom_border_color.customBorderColors = true;
				feature_custom_border_color.customBorderColorWithoutFormat = true;
				*next_feature = &feature_custom_border_color;
				next_feature = &feature_custom_border_color.pNext;

				if (use_mesh_shader)
				{
					VkPhysicalDeviceMaintenance4Features feature_maintenance4 = {};
					feature_maintenance4.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
					feature_maintenance4.maintenance4 = true;
					*next_feature = &feature_maintenance4;
					next_feature = &feature_maintenance4.pNext;
				}

				std::vector<const char*> required_device_extensions;
				required_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				required_device_extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
				required_device_extensions.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
				required_device_extensions.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
#if defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
				if (use_mesh_shader)
				{
					required_device_extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
					required_device_extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
				}
#endif

				VkDeviceCreateInfo device_info = {};
				device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				device_info.pNext = &enabled_features;
				device_info.pQueueCreateInfos = queue_infos.data();
				device_info.queueCreateInfoCount = queue_infos.size();
				device_info.enabledExtensionCount = required_device_extensions.size();
				device_info.ppEnabledExtensionNames = required_device_extensions.data();
				check_vk_result(vkCreateDevice(physical_device, &device_info, nullptr, &ret->vk_device));
				printf("vulkan: device created\n");
				device = ret;

				if (use_mesh_shader)
					vkCmdDrawMeshTasks = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(ret->vk_device, "vkCmdDrawMeshTasksEXT");

				graphics_command_pool.reset(graphics_queue_index != -1 ? CommandPool::create(graphics_queue_index) : nullptr);
				transfer_command_pool.reset(transfer_queue_index != -1 ? CommandPool::create(transfer_queue_index) : nullptr);
				graphics_queue.reset(graphics_queue_index != -1 ? QueuePrivate::create(graphics_queue_index) : nullptr);
				transfer_queue.reset(transfer_queue_index != -1 ? QueuePrivate::create(transfer_queue_index) : nullptr);
#endif
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

;