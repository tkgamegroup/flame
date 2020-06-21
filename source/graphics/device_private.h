#pragma once

#include <flame/graphics/device.h>
#include "graphics_private.h"

#include <flame/type.h>

namespace flame
{
	namespace graphics
	{
		struct DescriptorpoolPrivate;
		struct SamplerPrivate;
		struct CommandpoolPrivate;
		struct QueuePrivate;
		struct DevicePrivate;

		extern DevicePrivate* _default_device;

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
			std::unique_ptr<DescriptorpoolPrivate> default_descriptorpool;
			std::unique_ptr<SamplerPrivate> default_sampler_nearest;
			std::unique_ptr<SamplerPrivate> default_sampler_linear;
			std::unique_ptr<CommandpoolPrivate> default_graphics_commandpool;
			std::unique_ptr<CommandpoolPrivate> default_transfer_commandpool;
			std::unique_ptr<QueuePrivate> default_graphics_queue;
			std::unique_ptr<QueuePrivate> default_transfer_queue;

			DevicePrivate(bool debug);
			~DevicePrivate();

			bool _has_feature(Feature f) const;

			uint _find_memory_type(uint type_filter, MemPropFlags properties);

			void release() override { delete this; }
			void set_default() override { _default_device = this; }

			Descriptorpool* get_default_descriptorpool() const override { return default_descriptorpool.get(); }
			Sampler* get_default_sampler_nearest() const override { return default_sampler_nearest.get(); }
			Sampler* get_default_sampler_linear() const override { return default_sampler_linear.get(); }
			Commandpool* get_default_graphics_commandpool() const override { return default_graphics_commandpool.get(); }
			Commandpool* get_default_transfer_commandpool() const override { return default_transfer_commandpool.get(); }
			Queue* get_default_graphics_queue() const override { return default_graphics_queue.get(); }
			Queue* get_default_transfer_queue() const override { return default_transfer_queue.get(); }

			bool has_feature(Feature f) const override { return _has_feature(f); }
		};
	}
}
