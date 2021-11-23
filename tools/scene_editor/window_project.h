#pragma once

#include "app.h"

struct WindowProject : Window
{
	struct FolderTreeNode
	{
		bool read = false;
		std::filesystem::path path;
		std::vector<std::unique_ptr<FolderTreeNode>> children;

		std::string display_text;

		FolderTreeNode(const std::filesystem::path& path);

		void draw();
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

	std::unique_ptr<FolderTreeNode> foler_tree;
	FolderTreeNode* selected_folder = nullptr;

	std::map<int, std::unique_ptr<graphics::Image>> icons;
	std::vector<std::unique_ptr<graphics::Image>> thumbnails;
	std::vector<std::unique_ptr<Item>> items;
	bool _just_selected;

	WindowProject();
	void reset();
	void set_items_size(float size);

	void open_folder(const std::filesystem::path& path);

	void on_draw() override;
};

extern WindowProject window_project;
