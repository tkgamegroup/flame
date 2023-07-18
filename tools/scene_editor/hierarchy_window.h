#pragma once

#include "app.h"

struct HierarchyView : View
{
	std::string filter;
	std::string rename_string;
	uint rename_start_frame = 0;

	HierarchyView();
	HierarchyView(const std::string& name);
	~HierarchyView();
	void on_draw() override;
};

struct HierarchyWindow : Window
{
	HierarchyWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
};

extern HierarchyWindow hierarchy_window;
