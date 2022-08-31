#pragma once

#include "app.h"

struct View_Hierarchy : graphics::GuiView
{
	View_Hierarchy();

	void on_draw() override;
};

extern View_Hierarchy view_hierarchy;
