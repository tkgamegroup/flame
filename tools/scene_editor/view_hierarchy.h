#pragma once

#include "app.h"

struct View_Hierarchy : graphics::GuiView
{
	std::string filter;
	std::string rename_string;
	uint rename_start_frame = 0;

	View_Hierarchy();

	void on_draw() override;
};

extern View_Hierarchy view_hierarchy;
