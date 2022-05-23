#pragma once

#include "../foundation/application.h"
#include "device.h"
#include "image.h"
#include "renderpass.h"
#include "command.h"
#include "window.h"
#include "gui.h"

struct GraphicsApplication : Application
{
	graphics::WindowPtr main_window = nullptr;
	int render_frames = 0;
	bool always_render = true;

	void create(bool graphics_debug, std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags styles = WindowFrame | WindowResizable)
	{
		Application::create(title, size, styles);

		graphics::Device::create(graphics_debug);
		main_window = graphics::Window::create(Application::main_window);

		graphics::gui_set_current();
	}

	bool on_update() override
	{
		if (main_window->native->has_input)
			render_frames = 3;
		if (render_frames > 0 || always_render)
		{
			main_window->dirty = true;
			main_window->render();
			render_frames--;
		}
		return true;
	}
};
