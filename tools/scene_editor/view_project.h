#pragma once

#include "app.h"

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
		void set_size();

		void draw();
	};

	std::unique_ptr<FolderTreeNode> folder_tree;
	FolderTreeNode* selected_folder = nullptr;
	uint selected_folder_frame = 0;

	std::map<int, std::unique_ptr<graphics::Image>> icons;
	std::vector<std::unique_ptr<graphics::Image>> thumbnails;
	std::vector<std::unique_ptr<Item>> items;

	void* ev_watcher = nullptr;
	std::mutex mtx_changed_paths;
	std::vector<std::filesystem::path> changed_paths;

	View_Project();
	void reset();
	void set_items_size(float size);

	void select_folder(FolderTreeNode* folder);
	void open_folder(const std::filesystem::path& path);

	void on_draw() override;
};

extern View_Project view_project;
