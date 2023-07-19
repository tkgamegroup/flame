#pragma once

#include "app.h"

struct TimelineView : View
{
	std::vector<std::pair<int, int>> selected_keyframes;

	TimelineView();
	TimelineView(const std::string& name);
	void on_draw() override;
};

struct TimelineWindow : Window
{
	TimelineWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
};

extern TimelineWindow timeline_window;
