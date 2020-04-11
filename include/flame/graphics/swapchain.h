#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	struct SysWindow;
	struct BP;

	namespace graphics
	{
		struct Device;
		struct Image;
		struct Semaphore;

		struct Swapchain
		{
			FLAME_GRAPHICS_EXPORTS static Format get_format();

			FLAME_GRAPHICS_EXPORTS SysWindow* window() const;
			FLAME_GRAPHICS_EXPORTS uint image_count() const;
			FLAME_GRAPHICS_EXPORTS Image* image(uint idx) const;
			FLAME_GRAPHICS_EXPORTS Semaphore* image_avalible() const;

			FLAME_GRAPHICS_EXPORTS uint image_index() const;
			FLAME_GRAPHICS_EXPORTS void acquire_image();

			FLAME_GRAPHICS_EXPORTS uint hash() const;

			FLAME_GRAPHICS_EXPORTS static Swapchain *create(Device *d, SysWindow* w, bool add_trans_dst_usage = false);
			FLAME_GRAPHICS_EXPORTS static void destroy(Swapchain *s);
		};
	}
}

