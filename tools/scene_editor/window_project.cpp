#include "window_project.h"

WindowProject window_project;

WindowProject::FolderTreeNode::FolderTreeNode(const std::filesystem::path& path) :
	path(path)
{
}

void WindowProject::FolderTreeNode::draw()
{
	auto flags = window_project.selected_folder == this ? ImGuiTreeNodeFlags_Selected : 0;
	if (read && children.empty())
	{
		ImGui::TreeNodeEx(path.stem().string().c_str(), flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		if (ImGui::IsItemClicked())
		{
			window_project.selected_folder = this;
			window_project.open_folder(path);
		}
	}
	else
	{
		auto opened = ImGui::TreeNodeEx(path.stem().string().c_str(), flags | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
		if (ImGui::IsItemClicked())
		{
			window_project.selected_folder = this;
			window_project.open_folder(path);
		}
		if (opened)
		{
			if (!read)
			{
				for (auto& it : std::filesystem::directory_iterator(path))
				{
					if (std::filesystem::is_directory(it.status()))
						children.emplace_back(new FolderTreeNode(it.path()));
				}
				read = true;
			}
			for (auto& c : children)
				c->draw();
			ImGui::TreePop();
		}
	}
}

WindowProject::FileItem::FileItem(const std::filesystem::path& path) :
	path(path)
{
	auto ext = path.extension();
	if (is_image_file(ext))
	{
		uint w;
		uint h;
		uchar* d;
		get_thumbnail(64, path.c_str(), &w, &h, &d);
		auto img = graphics::Image::create(nullptr, graphics::Format_B8G8R8A8_UNORM, uvec2(w, h), d);
		thumbnail = img;
		window_project.thumbnails.emplace_back(img);
	}
	else
	{
		int icon_id;
		get_icon(path.c_str(), &icon_id, nullptr, nullptr, nullptr);
		auto it = window_project.icons.find(icon_id);
		if (it != window_project.icons.end())
			thumbnail = it->second.get();
		else
		{
			uint w;
			uint h;
			uchar* d;
			get_icon(path.c_str(), nullptr, &w, &h, &d);
			auto img = graphics::Image::create(nullptr, graphics::Format_B8G8R8A8_UNORM, uvec2(w, h), d);
			thumbnail = img;
			window_project.icons.emplace(icon_id, img);
		}
	}
}

void WindowProject::FileItem::draw()
{
	ImGui::Text(path.stem().string().c_str());
	ImGui::Image(thumbnail, ImVec2(64, 64));
}

WindowProject::WindowProject() :
	Window("Project")
{
}

void WindowProject::open_folder(const std::filesystem::path& path)
{
	graphics::Queue::get(nullptr)->wait_idle();

	file_items.clear();
	icons.clear();
	thumbnails.clear();

	for (auto& it : std::filesystem::directory_iterator(path))
		file_items.emplace_back(new FileItem(it.path()));
}

void WindowProject::on_draw()
{
	if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
	{
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("folders", ImVec2(0, -2));
		if (foler_tree)
			foler_tree->draw();
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
		ImGui::BeginChild("files", ImVec2(0, -2));
		for (auto& i : file_items)
			i->draw();
		ImGui::EndChild();

		ImGui::EndTable();
	}
}
