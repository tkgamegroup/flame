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
			VkInstance _instance;
			VkPhysicalDevice _physical_device;
			VkPhysicalDeviceProperties _props;
			VkPhysicalDeviceFeatures _features; 
			VkPhysicalDeviceMemoryProperties _mem_props;
			VkDevice _v;
			int _gq_idx;
			int _tq_idx;
#elif defined(FLAME_D3D12)
			IDXGIFactory4* factory; // just like instance
			IDXGIAdapter1* adapter; // just like physical device
			ID3D12Device4* v; // just like device
#endif
			std::unique_ptr<DescriptorpoolPrivate> _descriptorpool;
			std::unique_ptr<SamplerPrivate> _sampler_nearest;
			std::unique_ptr<SamplerPrivate> _sampler_linear;
			std::unique_ptr<CommandpoolPrivate> _graphics_commandpool;
			std::unique_ptr<CommandpoolPrivate> _transfer_commandpool;
			std::unique_ptr<QueuePrivate> _graphics_queue;
			std::unique_ptr<QueuePrivate> _transfer_queue;

			DevicePrivate(bool debug);
			~DevicePrivate();

			bool _has_feature(Feature f) const;

			uint _find_memory_type(uint type_filter, MemPropFlags properties);

			void release() override { delete this; }
			void set_default() override { _default_device = this; }

			Descriptorpool* get_descriptorpool() const override { return (Descriptorpool*)_descriptorpool.get(); }
			Sampler* get_sampler_nearest() const override { return (Sampler*)_sampler_nearest.get(); }
			Sampler* get_sampler_linear() const override { return (Sampler*)_sampler_linear.get(); }
			Commandpool* get_graphics_commandpool() const override { return (Commandpool*)_graphics_commandpool.get(); }
			Commandpool* get_transfer_commandpool() const override { return (Commandpool*)_transfer_commandpool.get(); }
			Queue* get_graphics_queue() const override { return (Queue*)_graphics_queue.get(); }
			Queue* get_transfer_queue() const override { return (Queue*)_transfer_queue.get(); }

			bool has_feature(Feature f) const override { return _has_feature(f); }
		};
	}
}
