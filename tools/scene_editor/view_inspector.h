#pragma once

#include "app.h"

struct View_Inspector : graphics::GuiView
{
	View_Inspector();

	void refresh();
	void on_draw() override;
};

extern View_Inspector view_inspector;
