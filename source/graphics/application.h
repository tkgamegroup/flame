#pragma once

#include "../foundation/application.h"
#include "device.h"
#include "image.h"
#include "renderpass.h"
#include "command.h"
#include "window.h"

struct GraphicsApplication : Application
{
	graphics::DevicePtr graphics_device = nullptr;
	graphics::WindowPtr main_window = nullptr;
	int render_frames = 0;
	bool always_render = true;

	void create(bool graphics_debug, std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags style = WindowFrame | WindowResizable)
	{
		graphics_device = graphics::Device::create(graphics_debug);
		Application::main_window = NativeWindow::create(title, size, style);
		main_window = graphics::Window::create(graphics_device, Application::main_window);
	}

	void run()
	{
		::run([this]() {
			if (main_window->native->has_input)
				render_frames = 3;
			if (render_frames > 0 || always_render)
			{
				main_window->dirty = true;
				main_window->imgui_new_frame();
				main_window->update();
			}
			render_frames--;
			return true;
		});
	}
};
