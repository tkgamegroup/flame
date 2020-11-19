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

			virtual bool has_feature(Feature f) const = 0;

			FLAME_GRAPHICS_EXPORTS static Device* get_default();
			FLAME_GRAPHICS_EXPORTS static void set_default(Device* device);
			FLAME_GRAPHICS_EXPORTS static Device* create(bool debug);
		};
	}
}

