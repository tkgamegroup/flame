#pragma once

#include "app.h"

struct TimelineWindow : graphics::GuiView
{
	std::vector<std::pair<int, int>> selected_keyframes;

	TimelineWindow();

	void on_draw() override;
};

extern TimelineWindow timeline_window;
