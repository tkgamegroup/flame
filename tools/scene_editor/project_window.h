#pragma once

#include "app.h"

#include <flame/foundation/system.h>
#include <flame/graphics/explorer_abstract.h>

struct ProjectView : View
{
	graphics::ExplorerAbstract explorer;

	ProjectView();
	ProjectView(const std::string& name);
	void on_draw() override;
};

struct ProjectWindow : Window
{
	void* flame_file_watcher = nullptr;
	void* assets_file_watcher = nullptr;
	void* code_file_watcher = nullptr;
	std::mutex mtx_changed_paths;
	std::map<std::filesystem::path, FileChangeFlags> changed_paths;

	ProjectWindow();
	void init() override;
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
	ProjectView* first_view() const;
	void reset();
	void ping(const std::filesystem::path& path);
	void process_changed_paths();
};

extern ProjectWindow project_window;
