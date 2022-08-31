#pragma once

#include "app.h"

struct View_Debugger : graphics::GuiView
{
	View_Debugger();

	void on_draw() override;
};

extern View_Debugger view_debugger;
