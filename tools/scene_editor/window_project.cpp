#include "selection.h"
#include "window_project.h"
#include "window_scene.h"

WindowProject window_project;

WindowProject::FolderTreeNode::FolderTreeNode(const std::filesystem::path& path) :
	path(path)
{
	display_text = path.stem().string();
}

void WindowProject::FolderTreeNode::draw()
{
	auto flags = window_project.selected_folder == this ? ImGuiTreeNodeFlags_Selected : 0;
	if (read && children.empty())
	{
		ImGui::TreeNodeEx(display_text.c_str(), flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		if (ImGui::IsItemClicked())
		{
			if (window_project.selected_folder != this)
			{
				window_project.selected_folder = this;
				window_project.open_folder(path);
			}
		}
	}
	else
	{
		auto opened = ImGui::TreeNodeEx(display_text.c_str(), flags | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
		if (ImGui::IsItemClicked())
		{
			if (window_project.selected_folder != this)
			{
				window_project.selected_folder = this;
				window_project.open_folder(path);
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
}

WindowProject::Item::Metric WindowProject::Item::metric = {};

WindowProject::Item::Item(const std::filesystem::path& path) :
	path(path)
{
	set_size();
}

void WindowProject::Item::set_size()
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
		uint w;
		uint h;
		uchar* d;
		get_thumbnail(metric.size, path.c_str(), &w, &h, &d);
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

void WindowProject::Item::draw()
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
		window_project._just_selected = true;
	}
	if (ImGui::IsMouseDoubleClicked(0) && active)
	{
		if (std::filesystem::is_directory(path))
		{
			// open folder will destroy all items, so staging path here
			auto str = path.wstring();
			add_event([](Capture& c) {
				window_project.open_folder((wchar_t*)c._data);
			}, Capture().set_data((str.size() + 1) * 2, (void*)str.c_str()));

			window_project.selected_folder = nullptr;
			std::function<FolderTreeNode*(FolderTreeNode*)> select_node;
			select_node = [&](FolderTreeNode* n)->FolderTreeNode* {
				if (n->path == path)
				{
					window_project.selected_folder = n;
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
			select_node(window_project.foler_tree.get());
		}
		else
		{
			auto ext = path.extension();
			if (ext == L".prefab")
				window_scene.open_prefab(path);
		}
	}
}

WindowProject::WindowProject() :
	Window("Project")
{
}

void WindowProject::reset()
{
	items.clear();
	selected_folder = nullptr;
	foler_tree.reset(new WindowProject::FolderTreeNode(app.project_path));
}

void WindowProject::set_items_size(float size)
{
	Item::metric.size = size;
	auto v = ImGui::GetStyle().FramePadding;
	Item::metric.padding = vec2(v.x, v.y);
	Item::metric.line_height = ImGui::GetTextLineHeight();

	for (auto& i : items)
		i->set_size();
}

void WindowProject::open_folder(const std::filesystem::path& path)
{
	if (Item::metric.size == 0)
		set_items_size(64);

	graphics::Queue::get(nullptr)->wait_idle();

	items.clear();
	icons.clear();
	thumbnails.clear();

	for (auto& it : std::filesystem::directory_iterator(path))
		items.emplace_back(new Item(it.path()));
}

void WindowProject::on_draw()
{
	if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
	{
		auto& style = ImGui::GetStyle();

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("folders", ImVec2(0, -2));
		if (foler_tree)
			foler_tree->draw();
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
