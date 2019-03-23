#pragma once

#include <flame/engine/ui/window.h>

struct HierarchyWindow : flame::ui::Window
{
	HierarchyWindow();
	~HierarchyWindow();
	virtual void on_show() override;
};

extern HierarchyWindow *hierarchy_window;
