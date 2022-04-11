#pragma once

#include "app.h"

#include <flame/foundation/system.h>

struct View_Project : View
{
	struct FolderTreeNode
	{
		bool read = false;
		std::filesystem::path path;
		FolderTreeNode* parent = nullptr;
		std::vector<std::unique_ptr<FolderTreeNode>> children;

		std::string display_text;

		FolderTreeNode(const std::filesystem::path& path);
		void read_children();
	};

	struct Item
	{
		struct Metric
		{
			float size;
			vec2 padding;
			float line_height;
		};
		static Metric metric;

		std::filesystem::path path;
		graphics::Image* thumbnail = nullptr;

		std::string display_text;
		float display_text_width;

		Item(const std::filesystem::path& path);

		void draw();
	};

	std::filesystem::path peeding_open_path;
	std::unique_ptr<FolderTreeNode> folder_tree;
	FolderTreeNode* opened_folder = nullptr;
	std::unique_ptr<FolderTreeNode> dialog_folder_tree;
	FolderTreeNode* dialog_opened_folder = nullptr;
	uint open_folder_frame = 0;
	std::vector<FolderTreeNode*> folder_history;
	int folder_history_idx = -1;

	std::map<int, std::unique_ptr<graphics::Image>> icons;
	std::vector<std::unique_ptr<graphics::Image>> thumbnails;
	std::vector<std::unique_ptr<Item>> items;

	void* ev_watcher = nullptr;
	std::mutex mtx_changed_paths;
	std::map<std::filesystem::path, FileChangeFlags> changed_paths;

	View_Project();
	void init() override;
	void reset(const std::filesystem::path& assets_path);

	FolderTreeNode* find_folder(const std::filesystem::path& path, bool force_read = false);
	void open_folder(FolderTreeNode* folder, bool from_histroy = false);

	void on_draw() override;
};

extern View_Project view_project;
