#pragma once

#include "device.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		extern DevicePtr device;

		struct DevicePrivate : Device
		{
			std::map<uint, uint> configs;

#if USE_D3D12
			IDXGIFactory4* dxgi_factory = nullptr;
			ID3D12Device* d3d12_device = nullptr;
			uint d3d12_rtv_size = 0;
			uint d3d12_dsv_size = 0;
			uint d3d12_srv_size = 0;
			uint d3d12_sp_size = 0;
#elif USE_VULKAN
			VkInstance vk_instance = 0;
			VkPhysicalDevice vk_physical_device = 0;
			VkPhysicalDeviceProperties2 vk_props;
			VkPhysicalDeviceDepthStencilResolveProperties vk_resolve_props;
#if defined(VK_VERSION_1_3) && VK_HEADER_VERSION >= 231
			VkPhysicalDeviceMeshShaderPropertiesEXT vk_meshshader_props;
#endif
			VkPhysicalDeviceFeatures vk_features;
			VkPhysicalDeviceMemoryProperties vk_mem_props;
			VkDevice vk_device = 0;
#endif

			~DevicePrivate();

			bool get_config(uint hash, uint& value) override;
			void set_object_debug_name(BufferPtr obj, const std::string& name) override;
			void set_object_debug_name(ImagePtr obj, const std::string& name) override;

#if USE_VULKAN
			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties) const;
#endif
		};
	}
}
