#pragma once

#include "app.h"

struct View_Timeline : graphics::GuiView
{
	std::vector<std::pair<int, int>> selected_keyframes;

	View_Timeline();

	void on_draw() override;
};

extern View_Timeline view_timeline;
