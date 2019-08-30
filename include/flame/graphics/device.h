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
			Descriptorpool* dp;
			Sampler* sp_nearest;
			Sampler* sp_linear;
			Commandpool* gcp; // graphics command pool
			Commandpool* tcp; // transfer command pool
			Queue* gq; // graphics queue
			Queue* tq; // transfer queue

			FLAME_GRAPHICS_EXPORTS bool has_feature(Feature f);

			FLAME_GRAPHICS_EXPORTS static Device* create(bool debug);
			FLAME_GRAPHICS_EXPORTS static void destroy(Device* d);
		};
	}
}

