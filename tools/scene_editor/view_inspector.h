#pragma once

#include "app.h"

struct View_Inspector : View
{
	std::vector<std::pair<Selection::Type, void* /*path or entity's instance id*/>> history;
	int histroy_idx = -1;

	View_Inspector();

	void on_draw() override;
};

extern View_Inspector view_inspector;
