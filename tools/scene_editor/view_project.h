#pragma once

#include "app.h"
#include "explorer_abstract.h"

#include <flame/foundation/system.h>

struct View_Project : View
{
	ExplorerAbstract explorer;

	void* ev_watcher = nullptr;
	std::mutex mtx_changed_paths;
	std::map<std::filesystem::path, FileChangeFlags> changed_paths;

	View_Project();
	void init() override;
	void reset(const std::filesystem::path& assets_path);

	void on_draw() override;
};

extern View_Project view_project;
