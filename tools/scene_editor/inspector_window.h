#pragma once

#include "app.h"

struct InspectorView : View
{
	bool dirty = true;

	InspectorView();
	InspectorView(const std::string& name);
	void on_draw() override;
};

struct InspectorWindow : Window
{
	InspectorWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
};

extern InspectorWindow inspector_window;
