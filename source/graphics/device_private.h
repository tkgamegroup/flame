#pragma once

#include "device.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate : Device
		{
			VkInstance vk_instance;
			VkPhysicalDevice vk_physical_device;
			VkPhysicalDeviceProperties vk_props;
			VkPhysicalDeviceFeatures vk_features; 
			VkPhysicalDeviceMemoryProperties vk_mem_props;
			VkDevice vk_device;

			std::vector<std::unique_ptr<ImagePrivate>> texs[2]; // no_srgb, srgb
			std::vector<std::unique_ptr<SamplerPrivate>> sps;
			std::unique_ptr<DescriptorPoolPrivate> dsp;
			std::vector<std::unique_ptr<RenderpassPrivate>> rps;
			std::vector<std::unique_ptr<DescriptorSetLayoutPrivate>> dsls;
			std::vector<std::unique_ptr<PipelineLayoutPrivate>> plls;
			std::vector<std::unique_ptr<ShaderPrivate>> sds;
			std::vector<std::unique_ptr<PipelinePrivate>> pls;
			std::unique_ptr<CommandPoolPrivate> gcp;
			std::unique_ptr<CommandPoolPrivate> tcp;
			std::unique_ptr<QueuePrivate> gq;
			std::unique_ptr<QueuePrivate> tq;

			DevicePrivate(bool debug);
			~DevicePrivate();

			uint find_memory_type(uint type_filter, MemoryPropertyFlags properties);

			void release() override { delete this; }

			bool has_feature(Feature feature) const override;
		};
	}
}
