#pragma once

#include "app.h"

struct SheetView : View
{
	std::filesystem::path sheet_path;
	SheetPtr sheet = nullptr;
	bool unsaved = false;

	SheetView();
	SheetView(const std::string& name);
	~SheetView();
	void on_draw() override;
};

struct SheetWindow : Window
{
	SheetWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern SheetWindow sheet_window;
