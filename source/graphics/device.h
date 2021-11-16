#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device
		{
			virtual ~Device() {}

			virtual bool has_feature(Feature f) const = 0;

			FLAME_GRAPHICS_EXPORTS static DevicePtr create(bool debug);
		};

		FLAME_GRAPHICS_EXPORTS extern DevicePtr default_device;
	}
}

