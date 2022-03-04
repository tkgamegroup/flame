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

	void create(bool graphics_debug, std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags styles = WindowFrame | WindowResizable)
	{
		Application::create(title, size, styles);

		graphics_device = graphics::Device::create(graphics_debug);
		main_window = graphics::Window::create(graphics_device, Application::main_window);

		ImGui::SetCurrentContext((ImGuiContext*)main_window->imgui_context());

#if USE_IM_FILE_DIALOG
		ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void*
		{
			return graphics::Image::create(nullptr, fmt == 1 ? graphics::Format_R8G8B8A8_UNORM : graphics::Format_B8G8R8A8_UNORM, uvec2(w, h), data);
		};

		ifd::FileDialog::Instance().DeleteTexture = [](void* tex)
		{
			add_event([tex]() {
				graphics::Queue::get(nullptr)->wait_idle();
				delete ((graphics::Image*)tex);
				return false;
			});
		};
#endif
	}

	bool on_update() override
	{
		if (main_window->native->has_input)
			render_frames = 3;
		if (render_frames > 0 || always_render)
		{
			main_window->dirty = true;
			main_window->imgui_new_frame();
			main_window->update();
			render_frames--;
		}
		return true;
	}
};
