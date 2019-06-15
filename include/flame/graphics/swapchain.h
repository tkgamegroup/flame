#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	struct Window;

	namespace graphics
	{
		struct Device;
		struct Image;
		struct Imageview;
		struct Semaphore;

		FLAME_GRAPHICS_EXPORTS Format$ get_swapchain_format();

		struct Swapchain
		{
			FLAME_GRAPHICS_EXPORTS Window *window() const;
			FLAME_GRAPHICS_EXPORTS int image_count() const;
			FLAME_GRAPHICS_EXPORTS Image *image(uint idx) const;
			FLAME_GRAPHICS_EXPORTS uint image_index() const;

			FLAME_GRAPHICS_EXPORTS void acquire_image(Semaphore *signal_semaphore);

			FLAME_GRAPHICS_EXPORTS static Swapchain *create(Device *d, Window *w);
			FLAME_GRAPHICS_EXPORTS static void destroy(Swapchain *s);
		};
	}
}

