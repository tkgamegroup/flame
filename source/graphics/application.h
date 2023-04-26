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
	bool use_gui = false;

	void create(std::string_view title, 
		const uvec2& size = uvec2(1280, 720), 
		WindowStyleFlags styles = WindowFrame | WindowResizable,
		bool _use_gui = false,
		bool graphics_debug = false, const std::vector<std::pair<uint, uint>>& graphics_configs = {})
	{
		Application::create(title, size, styles);

		graphics::Device::create(graphics_debug, graphics_configs);
		main_window = graphics::Window::create(Application::main_window);

		use_gui = _use_gui;
		if (use_gui)
		{
			graphics::gui_initialize();
			graphics::gui_set_current();
		}
	}

	virtual void on_render()
	{
		if (use_gui)
			graphics::gui_frame();
	}

	bool on_update() override
	{
		if (main_window->native->has_input)
			render_frames = 3;
		if (always_render)
			render_frames = max(render_frames, 1);
		if (render_frames > 0)
		{
			on_render();
			main_window->dirty = true;
			render_frames--;
		}
		main_window->render();
		return true;
	}
};
