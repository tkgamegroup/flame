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
			virtual void set_default() = 0;

			virtual DescriptorPool* get_descriptor_pool() const = 0;
			virtual Sampler* get_sampler_nearest() const = 0;
			virtual Sampler* get_sampler_linear() const = 0;
			virtual CommandPool* get_graphics_command_pool() const = 0;
			virtual CommandPool* get_transfer_command_pool() const = 0;
			virtual Queue* get_graphics_queue() const = 0;
			virtual Queue* get_transfer_queue() const = 0;

			virtual bool has_feature(Feature f) const = 0;

			FLAME_GRAPHICS_EXPORTS static Device* get_default();
			FLAME_GRAPHICS_EXPORTS static Device* create(bool debug, bool set_to_current = true);
		};
	}
}

