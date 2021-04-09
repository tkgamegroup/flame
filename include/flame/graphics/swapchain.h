#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Swapchain
		{
			virtual void release() = 0;

			virtual Window* get_window() const = 0;
			virtual uint get_images_count() const = 0;
			virtual ImagePtr get_image(uint idx) const = 0;
			virtual SemaphorePtr get_image_avalible() const = 0;

			virtual uint get_image_index() const = 0;
			virtual int acquire_image() = 0;

			FLAME_GRAPHICS_EXPORTS static Format get_format();

			FLAME_GRAPHICS_EXPORTS static Swapchain* create(Device* device, Window* window);
		};
	}
}
