#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Renderpass;
		struct Pipeline;
		struct Descriptorpool;
		struct Sampler;
		struct Commandpool;
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
			Descriptorpool* default_descriptorpool;
			Sampler* default_sampler_nearest;
			Sampler* default_sampler_linear;
			Commandpool* default_graphics_commandpool;
			Queue* default_graphics_queue;
			Commandpool* default_transfer_commandpool;
			Queue* default_transfer_queue;

			FLAME_GRAPHICS_EXPORTS bool has_feature(Feature f);

			FLAME_GRAPHICS_EXPORTS static Device* get_default();
			FLAME_GRAPHICS_EXPORTS static void set_default(Device* d);
			FLAME_GRAPHICS_EXPORTS static Device* create(bool debug, bool set_to_current = true);
			FLAME_GRAPHICS_EXPORTS static void destroy(Device* d);
		};
	}
}

