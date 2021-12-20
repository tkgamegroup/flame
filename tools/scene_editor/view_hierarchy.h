#pragma once

#include "app.h"

struct View_Hierarchy : View
{
	View_Hierarchy();

	bool _just_selected;

	void on_draw() override;
};

extern View_Hierarchy view_hierarchy;
