#pragma once

#include <flame/graphics/device.h>
#include "graphics_private.h"

#include <flame/type.h>

namespace flame
{
	namespace graphics
	{
		struct DescriptorPoolPrivate;
		struct SamplerPrivate;
		struct CommandPoolPrivate;
		struct QueuePrivate;
		struct DevicePrivate;

		extern DevicePrivate* _default_device;

		struct DevicePrivate : Device
		{
#if defined(FLAME_VULKAN)
			VkInstance vk_instance;
			VkPhysicalDevice vk_physical_device;
			VkPhysicalDeviceProperties vk_props;
			VkPhysicalDeviceFeatures vk_features; 
			VkPhysicalDeviceMemoryProperties vk_mem_props;
			VkDevice vk_device;
			int graphics_queue_index;
			int transfer_queue_index;
#elif defined(FLAME_D3D12)
			IDXGIFactory4* factory; // just like instance
			IDXGIAdapter1* adapter; // just like physical device
			ID3D12Device4* v; // just like device
#endif
			std::unique_ptr<DescriptorPoolPrivate> descriptor_pool;
			std::unique_ptr<SamplerPrivate> sampler_nearest;
			std::unique_ptr<SamplerPrivate> sampler_linear;
			std::unique_ptr<CommandPoolPrivate> graphics_command_pool;
			std::unique_ptr<CommandPoolPrivate> transfer_command_pool;
			std::unique_ptr<QueuePrivate> graphics_queue;
			std::unique_ptr<QueuePrivate> transfer_queue;

			DevicePrivate(bool debug);
			~DevicePrivate();

			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties);

			void release() override { delete this; }
			void set_default() override { _default_device = this; }

			DescriptorPool* get_descriptor_pool() const override { return (DescriptorPool*)descriptor_pool.get(); }
			Sampler* get_sampler_nearest() const override { return (Sampler*)sampler_nearest.get(); }
			Sampler* get_sampler_linear() const override { return (Sampler*)sampler_linear.get(); }
			CommandPool* get_graphics_command_pool() const override { return (CommandPool*)graphics_command_pool.get(); }
			CommandPool* get_transfer_command_pool() const override { return (CommandPool*)transfer_command_pool.get(); }
			Queue* get_graphics_queue() const override { return (Queue*)graphics_queue.get(); }
			Queue* get_transfer_queue() const override { return (Queue*)transfer_queue.get(); }

			bool has_feature(Feature f) const override;
		};
	}
}
