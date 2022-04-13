#pragma once

#include "app.h"

struct ResourcePanel
{
	struct FolderTreeNode
	{
		ResourcePanel* panel;

		bool read = false;
		std::filesystem::path path;
		FolderTreeNode* parent = nullptr;
		std::vector<std::unique_ptr<FolderTreeNode>> children;

		std::string display_text;

		bool peeding_open = false;

		FolderTreeNode(ResourcePanel* panel, const std::filesystem::path& path);
		void read_children();
		void mark_upstream_open();
		void draw();
	};

	struct Item
	{
		struct Metric
		{
			float size;
			vec2 padding;
			float line_height;
			void init();
		};
		static Metric metric;

		ResourcePanel* panel;

		std::filesystem::path path;
		std::string text;
		float text_width;
		graphics::ImagePtr image = nullptr;

		bool has_children = false;

		Item(ResourcePanel* panel, const std::filesystem::path& path, const std::string& text, graphics::ImagePtr image);
		Item(ResourcePanel* panel, const std::filesystem::path& path);
		void prune_text();

		bool draw();
	};

	std::unique_ptr<FolderTreeNode> folder_tree;
	FolderTreeNode* opened_folder = nullptr;
	uint open_folder_frame = 0;
	std::filesystem::path peeding_open_path;
	std::pair<FolderTreeNode*, bool> peeding_open_node;
	FolderTreeNode* peeding_scroll_here_folder = nullptr;
	std::vector<FolderTreeNode*> folder_history;
	int folder_history_idx = -1;

	std::vector<std::unique_ptr<graphics::Image>> thumbnails;
	std::vector<std::unique_ptr<Item>> items;
	std::filesystem::path selected_path;

	std::function<void(const std::filesystem::path&)> select_callback;
	std::function<void(const std::filesystem::path&)> dbclick_callback;
	std::function<void(const std::filesystem::path&)> item_context_menu_callback;
	std::function<void(const std::filesystem::path&)> folder_context_menu_callback;

	void reset(const std::filesystem::path& path);
	FolderTreeNode* find_folder(const std::filesystem::path& path, bool force_read = false);
	void open_folder(FolderTreeNode* folder, bool from_histroy = false);
	void ping(const std::filesystem::path& path);
	void draw();
};
