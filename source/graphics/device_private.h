#include <flame/graphics/device.h>
#include "graphics_private.h"

#include <flame/type.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate : Device
		{
#if defined(FLAME_VULKAN)
			VkInstance instance;
			VkPhysicalDevice physical_device;
			VkPhysicalDeviceProperties props;
			VkPhysicalDeviceFeatures features; 
			VkPhysicalDeviceMemoryProperties mem_props;
			VkDevice v;
			int gq_idx;
			int tq_idx;
#elif defined(FLAME_D3D12)
			IDXGIFactory4* factory; // just like instance
			IDXGIAdapter1* adapter; // just like physical device
			ID3D12Device4* v; // just like device
#endif

			DevicePrivate(bool debug);
			~DevicePrivate();

			uint find_memory_type(uint type_filter, MemProp$ properties);
			bool has_feature(Feature f);
		};
	}
}
