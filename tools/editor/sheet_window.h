#pragma once

#include "app.h"

struct SheetView : View
{
	std::filesystem::path sheet_path;
	SheetPtr sheet = nullptr;
	bool vertical_mode = false;
	bool unsaved = false;

	SheetView();
	SheetView(const std::string& name);
	~SheetView();

	void save_sheet();
	void on_draw() override;
	std::string get_save_name() override;
};

struct SheetWindow : Window
{
	SheetWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern SheetWindow sheet_window;
