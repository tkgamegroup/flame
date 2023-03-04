#pragma once

#include "device.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		extern PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectName;

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

			bool get_config(uint hash, uint& value) override;
			void set_object_debug_name(BufferPtr obj, const std::string& name) override;
			void set_object_debug_name(ImagePtr obj, const std::string& name) override;
			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties);
		};
	}
}
