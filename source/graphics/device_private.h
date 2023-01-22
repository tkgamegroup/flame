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

			VkInstance vk_instance;
			VkPhysicalDevice vk_physical_device;
			VkPhysicalDeviceProperties2 vk_props;
			VkPhysicalDeviceDepthStencilResolveProperties vk_resolve_props;
			VkPhysicalDeviceMeshShaderPropertiesEXT vk_meshshader_props;
			VkPhysicalDeviceFeatures vk_features; 
			VkPhysicalDeviceMemoryProperties vk_mem_props;
			VkDevice vk_device;

			uint get_config(uint hash) override;
			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties);
		};
	}
}
