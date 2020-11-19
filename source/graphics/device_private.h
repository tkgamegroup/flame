#pragma once

#include <flame/graphics/device.h>
#include "graphics_private.h"

#include <flame/type.h>

namespace flame
{
	namespace graphics
	{
		struct SamplerPrivate;
		struct DescriptorPoolPrivate;
		struct DescriptorSetLayoutPrivate;
		struct PipelineLayoutPrivate;
		struct ShaderPrivate;
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

			std::vector<std::unique_ptr<SamplerPrivate>> sps;
			std::unique_ptr<DescriptorPoolPrivate> dsp;
			std::vector<std::unique_ptr<DescriptorSetLayoutPrivate>> dsls;
			std::vector<std::unique_ptr<PipelineLayoutPrivate>> plls;
			std::vector<std::pair<uint, std::unique_ptr<ShaderPrivate>>> sds;
			std::unique_ptr<CommandPoolPrivate> gcp;
			std::unique_ptr<CommandPoolPrivate> tcp;
			std::unique_ptr<QueuePrivate> gq;
			std::unique_ptr<QueuePrivate> tq;

			DevicePrivate(bool debug);
			~DevicePrivate();

			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties);

			void release() override { delete this; }

			bool has_feature(Feature f) const override;
		};
	}
}
