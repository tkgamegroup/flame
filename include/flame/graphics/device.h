#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct DescriptorPool;
		struct Sampler;
		struct CommandPool;
		struct Queue;

		enum Feature
		{
			FeatureTextureCompressionBC,
			FeatureTextureCompressionASTC_LDR,
			FeatureTextureCompressionETC2,

			FeatureCount
		};

		struct Device
		{
			virtual void release() = 0;

			virtual DescriptorPool* get_descriptor_pool() const = 0;
			virtual Sampler* get_sampler(Filter filter) const = 0;
			virtual CommandPool* get_command_pool(QueueFamily family) const = 0;
			virtual Queue* get_queue(QueueFamily family) const = 0;

			virtual bool has_feature(Feature f) const = 0;

			FLAME_GRAPHICS_EXPORTS static Device* get();
			FLAME_GRAPHICS_EXPORTS static Device* create(bool debug, bool as_default = true);
		};
	}
}

