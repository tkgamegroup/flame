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

		extern thread_local DevicePrivate* default_device;

		struct DevicePrivate : Device
		{
			VkInstance vk_instance;
			VkPhysicalDevice vk_physical_device;
			VkPhysicalDeviceProperties vk_props;
			VkPhysicalDeviceFeatures vk_features; 
			VkPhysicalDeviceMemoryProperties vk_mem_props;
			VkDevice vk_device;

			std::unique_ptr<DescriptorPoolPrivate> dsp;
			std::unique_ptr<SamplerPrivate> nsp;
			std::unique_ptr<SamplerPrivate> lsp;
			std::unique_ptr<CommandPoolPrivate> gcp;
			std::unique_ptr<CommandPoolPrivate> tcp;
			std::unique_ptr<QueuePrivate> gq;
			std::unique_ptr<QueuePrivate> tq;

			DevicePrivate(bool debug);
			~DevicePrivate();

			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties);

			void release() override { delete this; }

			CommandPool* get_command_pool(QueueFamily family) const override
			{
				switch (family)
				{
				case QueueGraphics:
					return (CommandPool*)gcp.get();
				case QueueTransfer:
					return (CommandPool*)tcp.get();
				}
				return nullptr;
			}
			Queue* get_queue(QueueFamily family) const override
			{
				switch (family)
				{
				case QueueGraphics:
					return (Queue*)gq.get();
				case QueueTransfer:
					return (Queue*)tq.get();
				}
				return nullptr;
			}

			bool has_feature(Feature f) const override;
		};
	}
}
