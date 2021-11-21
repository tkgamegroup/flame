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

			struct Create
			{
				virtual DevicePtr operator()(bool debug) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Create& create;

			struct Current
			{
				virtual DevicePtr& operator()() = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Current& current;
		};
	}
}

