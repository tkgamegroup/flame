#pragma once

#include "app.h"

struct View_Inspector : graphics::GuiView
{
	View_Inspector();

	void on_draw() override;

	static void clear_typeinfos();
};

extern View_Inspector view_inspector;
