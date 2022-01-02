#include "selection.h"
#include "view_project.h"
#include "view_scene.h"

#include <flame/foundation/system.h>

View_Project view_project;

View_Project::FolderTreeNode::FolderTreeNode(const std::filesystem::path& path) :
	path(path)
{
	display_text = path.stem().string();
}

void View_Project::FolderTreeNode::draw()
{
	auto flags = view_project.selected_folder == this ? ImGuiTreeNodeFlags_Selected : 0;
	if (read && children.empty())
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	else
		flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	auto opened = ImGui::TreeNodeEx(display_text.c_str(), flags) && !(flags & ImGuiTreeNodeFlags_Leaf);
	if (ImGui::IsItemClicked())
	{
		if (view_project.selected_folder != this)
		{
			view_project.selected_folder = this;
			view_project.open_folder(path);
		}
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

View_Project::Item::Metric View_Project::Item::metric = {};

View_Project::Item::Item(const std::filesystem::path& path) :
	path(path)
{
	set_size();
}

void View_Project::Item::set_size()
{
	display_text = path.filename().string();

	auto font = ImGui::GetFont();
	auto font_size = ImGui::GetFontSize();
	const char* clipped_end;
	display_text_width = font->CalcTextSizeA(font_size, metric.size, 0.f, &*display_text.begin(), display_text.c_str() + display_text.size(), &clipped_end).x;
	if (clipped_end != display_text.c_str() + display_text.size())
	{
		auto str = display_text.substr(0, clipped_end - display_text.c_str());
		float w;
		do
		{
			if (str.size() <= 1)
				break;
			str.pop_back();
			w = font->CalcTextSizeA(font_size, 9999.f, 0.f, (str + "...").c_str()).x;
		} while (w > metric.size);
		display_text = str + "...";
		display_text_width = w;
	}

	if (is_image_file(path.extension()))
	{
		auto d = get_thumbnail(metric.size, path.c_str());
		auto img = graphics::Image::create(nullptr, graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
		thumbnail = img;
		view_project.thumbnails.emplace_back(img);
	}
	else
	{
		int icon_id;
		get_icon(path.c_str(), &icon_id);
		auto it = view_project.icons.find(icon_id);
		if (it != view_project.icons.end())
			thumbnail = it->second.get();
		else
		{
			auto d = get_icon(path.c_str(), nullptr);
			auto img = graphics::Image::create(nullptr, graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
			thumbnail = img;
			view_project.icons.emplace(icon_id, img);
		}
	}
}

void View_Project::Item::draw()
{
	ImGui::BeginGroup();
	auto pressed = ImGui::InvisibleButton("", ImVec2(metric.size + metric.padding.x * 2, metric.size + metric.line_height + metric.padding.y * 3));
	auto draw_list = ImGui::GetWindowDrawList();
	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	auto active = ImGui::IsItemActive();
	auto hovered = ImGui::IsItemHovered();
	ImU32 col;
	if		(active)								col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if (hovered || selection.selecting(path))	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	else											col = ImColor(0, 0, 0, 0);
	draw_list->AddRectFilled(p0, p1, col);
	draw_list->AddImage(thumbnail, ImVec2(p0.x + metric.padding.x, p0.y + metric.padding.y), ImVec2(p1.x - metric.padding.x, p1.y - metric.line_height - metric.padding.y * 2));
	draw_list->AddText(ImVec2(p0.x + metric.padding.x + (metric.size - display_text_width) / 2, p0.y + metric.size + metric.padding.y * 2), ImColor(255, 255, 255), display_text.c_str(), display_text.c_str() + display_text.size());
	ImGui::EndGroup();

	if (pressed)
	{
		selection.select(path);
		view_project._just_selected = true;
	}
	if (ImGui::IsMouseDoubleClicked(0) && active)
	{
		if (std::filesystem::is_directory(path))
		{
			// open folder will destroy all items, so stage path here
			auto str = path.wstring();
			add_event([str]() {
				view_project.open_folder(str);
				return false;
			});

			view_project.selected_folder = nullptr;
			std::function<FolderTreeNode*(FolderTreeNode*)> select_node;
			select_node = [&](FolderTreeNode* n)->FolderTreeNode* {
				if (n->path == path)
				{
					view_project.selected_folder = n;
					return n;
				}
				for (auto& c : n->children)
				{
					auto ret = select_node(c.get());
					if (ret)
						return ret;
				}
				return nullptr;
			};
			select_node(view_project.folder_tree.get());
		}
		else
		{
			auto ext = path.extension();
			if (ext == L".prefab")
				view_scene.open_prefab(path);
		}
	}
}

View_Project::View_Project() :
	View("Project")
{
}

void View_Project::reset()
{
	items.clear();
	selected_folder = nullptr;
	folder_tree.reset(new View_Project::FolderTreeNode(app.project_path));
}

void View_Project::set_items_size(float size)
{
	Item::metric.size = size;
	auto v = ImGui::GetStyle().FramePadding;
	Item::metric.padding = vec2(v.x, v.y);
	Item::metric.line_height = ImGui::GetTextLineHeight();

	for (auto& i : items)
		i->set_size();
}

void View_Project::open_folder(const std::filesystem::path& path)
{
	if (Item::metric.size == 0)
		set_items_size(64);

	graphics::Queue::get(nullptr)->wait_idle();

	items.clear();
	icons.clear();
	thumbnails.clear();

	for (auto& it : std::filesystem::directory_iterator(path))
		items.emplace_back(new Item(it.path()));
	std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
		return a->path < b->path;
	});
}

void View_Project::on_draw()
{
	if (!selected_folder && !app.project_path.empty())
	{
		open_folder(app.project_path);
		selected_folder = folder_tree.get();
	}
	if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
	{
		auto& style = ImGui::GetStyle();

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("folders", ImVec2(0, -2));
		if (folder_tree)
			folder_tree->draw();
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1); 
		_just_selected = false;
		ImGui::BeginChild("files", ImVec2(0, -ImGui::GetFontSize() - style.ItemSpacing.y * 2));
		if (!items.empty())
		{
			auto window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			auto spacing = style.ItemSpacing.x;
			auto item_size = Item::metric.size + Item::metric.padding.x * 2;
			for (auto i = 0; i < items.size(); i++)
			{
				ImGui::PushID(i);
				items[i]->draw();
				ImGui::PopID();
				float next_x2 = ImGui::GetItemRectMax().x + spacing + item_size;
				if (i + 1 < items.size() && next_x2 < window_visible_x2)
					ImGui::SameLine();
			}
		}
		if (ImGui::IsMouseReleased(0) && ImGui::IsWindowFocused() && !_just_selected)
			selection.clear();
		ImGui::EndChild();
		if (selection.type == Selection::tFile)
			ImGui::TextUnformatted(selection.path.string().c_str());

		ImGui::EndTable();
	}
}
