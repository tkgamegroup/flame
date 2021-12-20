#pragma once

#include "window.h"

using namespace flame;

struct Application
{
	NativeWindowPtr main_window = nullptr;

	virtual ~Application() {}

	void create(std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags styles = WindowFrame | WindowResizable)
	{
		main_window = NativeWindow::create(title, size, styles);
	}

	virtual bool on_update() { return true; }

	void run()
	{
		::run([this]() {
			return on_update();
		});
	}
};
