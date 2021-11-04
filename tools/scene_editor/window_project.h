#pragma once

#include "app.h"

struct WindowProject : Window
{
	struct FolderTreeNode
	{
		bool read = false;
		std::filesystem::path path;
		std::vector<std::unique_ptr<FolderTreeNode>> children;

		FolderTreeNode(const std::filesystem::path& path);

		void draw();
	};

	struct FileItem
	{
		std::filesystem::path path;
		graphics::Image* thumbnail = nullptr;

		FileItem(const std::filesystem::path& path);

		void draw();
	};

	std::unique_ptr<FolderTreeNode> foler_tree;
	FolderTreeNode* selected_folder = nullptr;

	std::map<int, UniPtr<graphics::Image>> icons;
	std::vector<UniPtr<graphics::Image>> thumbnails;
	std::vector<std::unique_ptr<FileItem>> file_items;

	WindowProject();

	void open_folder(const std::filesystem::path& path);

	void on_draw() override;
};

extern WindowProject window_project;
