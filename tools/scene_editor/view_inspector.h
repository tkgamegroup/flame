#pragma once

#include "app.h"

struct View_Inspector : View
{
	View_Inspector();

	void on_draw() override;
};

extern View_Inspector view_inspector;
