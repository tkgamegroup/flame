#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device
		{
			virtual ~Device() {}

			virtual bool get_config(uint hash, uint& value) = 0;
			virtual void set_object_debug_name(BufferPtr obj, const std::string& name) = 0;
			virtual void set_object_debug_name(ImagePtr obj, const std::string& name) = 0;

			struct Create
			{
				// configs:
				//		mesh_shader: 0(OFF), 1(ON, default)
				//		replace_renderpass_attachment_dont_care_to_load: 0(OFF, default), 1(ON)
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

