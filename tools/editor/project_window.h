#pragma once

#include "app.h"

#include <flame/foundation/system.h>
#include <flame/graphics/explorer.h>

struct ProjectView : View
{
	graphics::Explorer explorer;

	ProjectView();
	ProjectView(const std::string& name);
	void on_draw() override;
	void reset();
};

struct ProjectWindow : Window
{
	std::filesystem::path flame_path;
	std::filesystem::path assets_path;
	std::filesystem::path code_path;

	void* flame_file_watcher = nullptr;
	void* assets_file_watcher = nullptr;
	void* code_file_watcher = nullptr;
	std::mutex mtx_changed_paths;
	std::map<std::filesystem::path, FileChangeFlags> changed_paths;

	ProjectWindow();
	void init() override;
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
	ProjectView* first_view() const;
	void reset();
	void ping(const std::filesystem::path& path);
	void process_changed_paths();
};

extern ProjectWindow project_window;
