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

	void create(std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags style = WindowFrame | WindowResizable)
	{
		graphics_device = graphics::Device::create(true);
		Application::main_window = NativeWindow::create(title, size, style);
		main_window = graphics::Window::create(graphics_device, Application::main_window);
	}

	void run()
	{
		::run([this]() {
			main_window->dirty = true;
			main_window->imgui_new_frame();
			main_window->update();
			return true;
		});
	}
};
