#pragma once

#include "app.h"

#include <flame/foundation/system.h>

struct View_Project : View
{
	enum Icon
	{
		Icon_Model,
		Icon_Armature,
		Icon_Mesh,

		IconCount
	};

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
		std::string text;
		float text_width;
		graphics::ImagePtr image = nullptr;

		bool has_children = false;

		Item(const std::filesystem::path& path, const std::string& text, graphics::ImagePtr image);
		Item(const std::filesystem::path& path);
		void prune_text();

		void draw();
	};

	std::filesystem::path peeding_open_path;
	std::unique_ptr<FolderTreeNode> folder_tree;
	FolderTreeNode* opened_folder = nullptr;
	uint open_folder_frame = 0;
	std::vector<FolderTreeNode*> folder_history;
	int folder_history_idx = -1;

	std::vector<std::unique_ptr<graphics::Image>> thumbnails;
	graphics::ImagePtr icons[IconCount];
	std::map<int, std::unique_ptr<graphics::Image>> sys_icons;
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
