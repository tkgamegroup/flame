#pragma once

#include "app.h"

#include <flame/foundation/system.h>
#include <flame/graphics/explorer_abstract.h>

struct View_Project : graphics::GuiView
{
	graphics::ExplorerAbstract explorer;

	void* flame_file_watcher = nullptr;
	void* assets_file_watcher = nullptr;
	void* cpp_file_watcher = nullptr;
	std::mutex mtx_changed_paths;
	std::map<std::filesystem::path, FileChangeFlags> changed_paths;

	View_Project();
	void init() override;
	void reset();

	void on_draw() override;
};

extern View_Project view_project;
