#pragma once

#include "window.h"

using namespace flame;

struct Application
{
	NativeWindowPtr main_window = nullptr;

	virtual ~Application() {}

	void create(std::string_view title, const uvec2& size = uvec2(1280, 720), 
		WindowStyleFlags styles = WindowStyleFrame | WindowStyleResizable)
	{
		printf("pid: %d\n", getpid());
		process_events();
		main_window = NativeWindow::create(title, size, styles);
	}

	virtual bool on_update() { return true; }

	void run()
	{
		main_loop([this]() {
			return on_update();
		});
	}
};
