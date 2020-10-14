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

		extern DevicePrivate* default_device;

		struct DevicePrivate : Device
		{
			VkInstance vk_instance;
			VkPhysicalDevice vk_physical_device;
			VkPhysicalDeviceProperties vk_props;
			VkPhysicalDeviceFeatures vk_features; 
			VkPhysicalDeviceMemoryProperties vk_mem_props;
			VkDevice vk_device;
			int graphics_queue_index;
			int transfer_queue_index;
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

			DescriptorPool* get_descriptor_pool() const override { return (DescriptorPool*)descriptor_pool.get(); }
			Sampler* get_sampler(Filter filter) const override 
			{
				switch (filter)
				{
				case FilterNearest:
					return (Sampler*)sampler_nearest.get();
				case FilterLinear:
					return (Sampler*)sampler_linear.get();
				}
				return nullptr;
			}
			CommandPool* get_command_pool(QueueFamily family) const override
			{
				switch (family)
				{
				case QueueGraphics:
					return (CommandPool*)graphics_command_pool.get();
				case QueueTransfer:
					return (CommandPool*)transfer_command_pool.get();
				}
				return nullptr;
			}
			Queue* get_queue(QueueFamily family) const override
			{
				switch (family)
				{
				case QueueGraphics:
					return (Queue*)graphics_queue.get();
				case QueueTransfer:
					return (Queue*)transfer_queue.get();
				}
				return nullptr;
			}

			bool has_feature(Feature f) const override;
		};
	}
}
