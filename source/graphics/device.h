#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device
		{
			virtual ~Device() {}

			struct Create
			{
				// configs:
				//		mesh_shader: 0(OFF), 1(ON, default)
				virtual DevicePtr operator()(bool debug, const std::vector<std::pair<uint, uint>>& configs = {} /* first: hash, second: value */) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct Current
			{
				virtual DevicePtr operator()() = 0;
			};
			FLAME_GRAPHICS_API static Current& current;
		};
	}
}

