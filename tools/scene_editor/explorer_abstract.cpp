#include "explorer_abstract.h"

#include <flame/foundation/system.h>
#include <flame/graphics/model.h>

ExplorerAbstract::FolderTreeNode::FolderTreeNode(ExplorerAbstract* explorer, const std::filesystem::path& path) :
	explorer(explorer),
	path(path)
{
	display_text = path.filename().string();
}

void ExplorerAbstract::FolderTreeNode::read_children()
{
	if (read)
		return;

	children.clear();
	if (std::filesystem::is_directory(path))
	{
		for (auto& it : std::filesystem::directory_iterator(path))
		{
			if (std::filesystem::is_directory(it.status()) || it.path().extension() == L".fmod")
			{
				auto c = new FolderTreeNode(explorer, it.path());
				c->parent = this;
				children.emplace_back(c);
			}
		}
	}
	read = true;
}

void ExplorerAbstract::FolderTreeNode::mark_upstream_open()
{
	if (parent)
	{
		parent->peeding_open = true;
		parent->mark_upstream_open();
	}
}

void ExplorerAbstract::FolderTreeNode::draw()
{
	auto flags = explorer->opened_folder == this ? ImGuiTreeNodeFlags_Selected : 0;
	if (read && children.empty())
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	else
		flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	if (peeding_open)
	{
		ImGui::SetNextItemOpen(true);
		peeding_open = false;
	}
	auto opened = ImGui::TreeNodeEx(display_text.c_str(), flags) && !(flags & ImGuiTreeNodeFlags_Leaf);
	if (explorer->peeding_scroll_here_folder == this)
	{
		ImGui::SetScrollHereY();
		explorer->peeding_scroll_here_folder = nullptr;
	}
	if (ImGui::IsItemClicked())
	{
		if (explorer->opened_folder != this)
			explorer->open_folder(this);
	}
	if (opened)
	{
		read_children();
		for (auto& c : children)
			c->draw();
		ImGui::TreePop();
	}
}

ExplorerAbstract::Item::Metric ExplorerAbstract::Item::metric = {};
void ExplorerAbstract::Item::Metric::init()
{
	if (size == 0)
	{
		size = 64;
		padding = ImGui::GetStyle().FramePadding;
		line_height = ImGui::GetTextLineHeight();
	}
}

ExplorerAbstract::Item::Item(ExplorerAbstract* explorer, const std::filesystem::path& path, const std::string& text, graphics::ImagePtr image) :
	explorer(explorer),
	path(path),
	text(text),
	image(image)
{
	Item::metric.init();

	prune_text();
}

ExplorerAbstract::Item::Item(ExplorerAbstract* explorer, const std::filesystem::path& path) :
	explorer(explorer),
	path(path)
{
	Item::metric.init();

	text = path.filename().string();
	prune_text();

	auto ext = path.extension();
	if (ext == L".fmod")
		image = app.icons[Icon_Model];
	else if (is_image_file(ext))
	{
		auto d = get_thumbnail(metric.size, path);
		if (d.second)
		{
			auto img = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
			image = img;
			explorer->thumbnails.emplace_back(img);
		}
	}
	else
	{
		int id;
		get_icon(path.c_str(), &id);
		auto it = app.sys_icons.find(id);
		if (it != app.sys_icons.end())
			image = it->second.get();
		else
		{
			auto d = get_icon(path.c_str(), nullptr);
			if (d.second)
			{
				auto img = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
				image = img;
				app.sys_icons.emplace(id, img);
			}
		}
	}
}

void ExplorerAbstract::Item::prune_text()
{
	auto font = ImGui::GetFont();
	auto font_size = ImGui::GetFontSize();
	const char* clipped_end;
	text_width = font->CalcTextSizeA(font_size, metric.size, 0.f, &*text.begin(), text.c_str() + text.size(), &clipped_end).x;
	if (clipped_end != text.c_str() + text.size())
	{
		auto str = text.substr(0, clipped_end - text.c_str());
		float w;
		do
		{
			if (str.size() <= 1)
				break;
			str.pop_back();
			w = font->CalcTextSizeA(font_size, 9999.f, 0.f, (str + "...").c_str()).x;
		} while (w > metric.size);
		text = str + "...";
		text_width = w;
	}
}

bool ExplorerAbstract::Item::draw()
{
	auto selected = false;
	ImGui::InvisibleButton("", ImVec2(metric.size + metric.padding.x * 2, metric.size + metric.line_height + metric.padding.y * 3));
	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	auto hovered = ImGui::IsItemHovered();
	auto active = ImGui::IsItemActive();
	ImU32 col;
	if (active)											col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if (hovered || explorer->selected_path == path)	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	else												col = ImColor(0, 0, 0, 0);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(p0, p1, col);
	if (image)
		draw_list->AddImage(image, ImVec2(p0.x + metric.padding.x, p0.y + metric.padding.y), ImVec2(p1.x - metric.padding.x, p1.y - metric.line_height - metric.padding.y * 2));
	draw_list->AddText(ImVec2(p0.x + metric.padding.x + (metric.size - text_width) / 2, p0.y + metric.size + metric.padding.y * 2), ImColor(255, 255, 255), text.c_str(), text.c_str() + text.size());

	if (frames > explorer->open_folder_frame + 3 && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hovered && ImGui::IsItemDeactivated())
	{
		if (explorer->select_callback)
			explorer->select_callback(path);
		else
			explorer->selected_path = path;
		selected = true;
	}
	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && active)
	{
		if (has_children)
			explorer->peeding_open_path = path;
		else
		{
			if (explorer->dbclick_callback)
				explorer->dbclick_callback(path);
		}
	}

	if (ImGui::BeginDragDropSource())
	{
		auto str = path.wstring();
		ImGui::SetDragDropPayload("File", str.c_str(), sizeof(wchar_t) * (str.size() + 1));
		ImGui::TextUnformatted("File");
		ImGui::EndDragDropSource();
	}

	if (hovered)
		ImGui::SetTooltip("%s", path.filename().string().c_str());

	return selected;
}

void ExplorerAbstract::reset(const std::filesystem::path& path)
{
	items.clear();
	opened_folder = nullptr;
	folder_tree.reset(new FolderTreeNode(this, path));
}

ExplorerAbstract::FolderTreeNode* ExplorerAbstract::find_folder(const std::filesystem::path& path, bool force_read)
{
	std::function<FolderTreeNode* (FolderTreeNode*)> sub_find;
	sub_find = [&](FolderTreeNode* n)->FolderTreeNode* {
		if (n->path == path)
			return n;
		if (force_read)
			n->read_children();
		for (auto& c : n->children)
		{
			auto ret = sub_find(c.get());
			if (ret)
				return ret;
		}
		return nullptr;
	};
	return sub_find(folder_tree.get());
}

void ExplorerAbstract::open_folder(FolderTreeNode* folder, bool from_histroy)
{
	if (!folder)
		folder = folder_tree.get();

	if (!from_histroy && opened_folder != folder)
	{
		auto it = folder_history.begin() + (folder_history_idx + 1);
		it = folder_history.erase(it, folder_history.end());
		folder_history.insert(it, folder);
		if (folder_history.size() > 20)
			folder_history.erase(folder_history.begin());
		else
			folder_history_idx++;
	}

	opened_folder = folder;
	open_folder_frame = frames;

	items.clear();

	graphics::Queue::get()->wait_idle();
	thumbnails.clear();

	if (folder)
	{
		folder->mark_upstream_open();
		folder->read_children();

		if (std::filesystem::is_directory(folder->path))
		{
			std::vector<Item*> dirs;
			std::vector<Item*> files;
			for (auto& it : std::filesystem::directory_iterator(folder->path))
			{
				if (std::filesystem::is_directory(it.status()))
					dirs.push_back(new Item(this, it.path()));
				else
					files.push_back(new Item(this, it.path()));
			}
			std::sort(dirs.begin(), dirs.end(), [](const auto& a, const auto& b) {
				return a->path < b->path;
				});
			std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
				return a->path < b->path;
				});
			for (auto i : dirs)
			{
				i->has_children = true;
				items.emplace_back(i);
			}
			for (auto i : files)
			{
				if (i->path.extension() == L".fmod")
					i->has_children = true;
				items.emplace_back(i);
			}
		}
		else
		{
			auto ext = folder->path.extension();
			if (ext == L".fmod")
			{
				auto model = graphics::Model::get(folder->path);
				if (model)
				{
					if (!model->bones.empty())
					{
						auto path = folder->path;
						path += L"#armature";
						items.emplace_back(new Item(this, path, "armature", app.icons[Icon_Armature]));
					}
					for (auto i = 0; i < model->meshes.size(); i++)
					{
						auto path = folder->path;
						path += L"#" + wstr(i);
						items.emplace_back(new Item(this, path, str(i), app.icons[Icon_Mesh]));
					}
					graphics::Model::release(model);
				}
			}
		}
	}
}

void ExplorerAbstract::ping(const std::filesystem::path& path)
{
	std::filesystem::path p;
	auto sp = SUW::split(path.wstring(), '#');
	if (sp.size() > 1)
		p = sp.front();
	else
		p = path.parent_path();
	auto folder = find_folder(p, true);
	if (folder)
	{
		folder->mark_upstream_open();
		peeding_scroll_here_folder = folder;
		open_folder(folder);
	}
}

void ExplorerAbstract::draw()
{
	if (!peeding_open_path.empty())
	{
		open_folder(find_folder(peeding_open_path, true));
		peeding_open_path.clear();
	}
	if (peeding_open_node.first)
	{
		open_folder(peeding_open_node.first, true);
		peeding_open_node = { nullptr, false };
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
		if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-left"_h).c_str()))
		{
			if (folder_history_idx > 0)
			{
				folder_history_idx--;
				peeding_open_node = { folder_history[folder_history_idx], true };
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-right"_h).c_str()))
		{
			if (folder_history_idx + 1 < folder_history.size())
			{
				folder_history_idx++;
				peeding_open_node = { folder_history[folder_history_idx], true };
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-up"_h).c_str()))
		{
			if (opened_folder && opened_folder->parent)
				peeding_open_node = { opened_folder->parent, false };
		}
		if (opened_folder)
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(Path::reverse(opened_folder->path).string().c_str());
		}

		Item::metric.init();
		auto content_size = ImGui::GetContentRegionAvail();
		content_size.y -= 4;
		if (ImGui::BeginTable("contents", content_size.x / (Item::metric.size + Item::metric.padding.x * 2 + style.ItemSpacing.x), ImGuiTableFlags_ScrollY, content_size))
		{
			auto just_selected = false;
			if (!items.empty())
			{
				for (auto i = 0; i < items.size(); i++)
				{
					auto& item = items[i];

					ImGui::TableNextColumn();
					ImGui::PushID(i);
					if (item->draw())
						just_selected = true;
					ImGui::PopID();

					if (item_context_menu_callback)
					{
						if (ImGui::BeginPopupContextItem())
						{
							item_context_menu_callback(item->path);
							ImGui::EndPopup();
						}
					}
				}
			}
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !just_selected)
			{
				if (select_callback)
					select_callback(L"");
				else
					selected_path = L"";
			}
			if (opened_folder && folder_context_menu_callback)
			{
				if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
				{
					folder_context_menu_callback(opened_folder->path);
					ImGui::EndPopup();
				}
			}
			ImGui::EndTable();
		}

		ImGui::EndTable();
	}
}
