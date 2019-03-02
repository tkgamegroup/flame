#pragma once

#include "material_shower.h"

struct InspectorWindow : flame::ui::Window
{
	std::unique_ptr<MaterialShower> material_shower;

	InspectorWindow();
	~InspectorWindow();
	virtual void on_show() override;
};

extern InspectorWindow *inspector_window;
