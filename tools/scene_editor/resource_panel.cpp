#include "resource_panel.h"

#include <flame/foundation/system.h>
#include <flame/graphics/model.h>

ResourcePanel::FolderTreeNode::FolderTreeNode(ResourcePanel* panel, const std::filesystem::path& path) :
	panel(panel),
	path(path)
{
	display_text = path.filename().string();
}

void ResourcePanel::FolderTreeNode::read_children()
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
				auto c = new FolderTreeNode(panel, it.path());
				c->parent = this;
				children.emplace_back(c);
			}
		}
	}
	read = true;
}

void ResourcePanel::FolderTreeNode::mark_upstream_open()
{
	if (parent)
	{
		parent->peeding_open = true;
		parent->mark_upstream_open();
	}
}

void ResourcePanel::FolderTreeNode::draw()
{
	auto flags = panel->opened_folder == this ? ImGuiTreeNodeFlags_Selected : 0;
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
	if (panel->peeding_scroll_here_folder == this)
	{
		ImGui::SetScrollHereY();
		panel->peeding_scroll_here_folder = nullptr;
	}
	if (ImGui::IsItemClicked())
	{
		if (panel->opened_folder != this)
			panel->open_folder(this);
	}
	if (opened)
	{
		read_children();
		for (auto& c : children)
			c->draw();
		ImGui::TreePop();
	}
}

ResourcePanel::Item::Metric ResourcePanel::Item::metric = {};

ResourcePanel::Item::Item(ResourcePanel* panel, const std::filesystem::path& path, const std::string& text, graphics::ImagePtr image) :
	panel(panel),
	path(path),
	text(text),
	image(image)
{
	prune_text();
}

ResourcePanel::Item::Item(ResourcePanel* panel, const std::filesystem::path& path) :
	panel(panel),
	path(path)
{
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
			panel->thumbnails.emplace_back(img);
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

void ResourcePanel::Item::prune_text()
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

bool ResourcePanel::Item::draw()
{
	auto selected = false;
	ImGui::InvisibleButton("", ImVec2(metric.size + metric.padding.x * 2, metric.size + metric.line_height + metric.padding.y * 3));
	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	auto hovered = ImGui::IsItemHovered();
	auto active = ImGui::IsItemActive();
	ImU32 col;
	if (active)											col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if (hovered || panel->selecting_path == path)	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	else												col = ImColor(0, 0, 0, 0);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(p0, p1, col);
	if (image)
		draw_list->AddImage(image, ImVec2(p0.x + metric.padding.x, p0.y + metric.padding.y), ImVec2(p1.x - metric.padding.x, p1.y - metric.line_height - metric.padding.y * 2));
	draw_list->AddText(ImVec2(p0.x + metric.padding.x + (metric.size - text_width) / 2, p0.y + metric.size + metric.padding.y * 2), ImColor(255, 255, 255), text.c_str(), text.c_str() + text.size());

	if (frames > panel->open_folder_frame + 3 && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hovered && ImGui::IsItemDeactivated())
	{
		if (panel->select_callback)
			panel->select_callback(path);
		selected = true;
	}
	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && active)
	{
		if (panel->dbclick_callback)
			panel->dbclick_callback(path, has_children);
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

void ResourcePanel::init()
{
	Item::metric.size = 64;
	Item::metric.padding = ImGui::GetStyle().FramePadding;
	Item::metric.line_height = ImGui::GetTextLineHeight();
}

void ResourcePanel::reset(const std::filesystem::path& path)
{
	items.clear();
	opened_folder = nullptr;
	folder_tree.reset(new FolderTreeNode(this, path));
}

ResourcePanel::FolderTreeNode* ResourcePanel::find_folder(const std::filesystem::path& path, bool force_read)
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

void ResourcePanel::open_folder(FolderTreeNode* folder, bool from_histroy)
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
				auto model = graphics::Model::get_stat(folder->path);
				if (model)
				{
					if (!model->bones.empty())
					{
						auto path = folder->path;
						path += L":armature";
						items.emplace_back(new Item(this, path, "armature", app.icons[Icon_Armature]));
					}
					for (auto i = 0; i < model->meshes.size(); i++)
					{
						auto path = folder->path;
						path += L":" + wstr(i);
						items.emplace_back(new Item(this, path, str(i), app.icons[Icon_Mesh]));
					}
				}
			}
		}
	}
}

void ResourcePanel::ping(const std::filesystem::path& path)
{
	assert(path.is_absolute());
	std::filesystem::path p;
	auto sp = SUW::split(path.wstring(), ':');
	if (sp.size() == 3)
		p = sp[0] + L":\\" + sp[1];
	else
		p = path.parent_path();
	auto folder = find_folder(p, true);
	folder->mark_upstream_open();
	peeding_scroll_here_folder = folder;
	open_folder(folder);
}

void ResourcePanel::draw()
{
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
			add_event([this]() {
				if (folder_history_idx > 0)
				{
					folder_history_idx--;
					open_folder(folder_history[folder_history_idx], true);
				}
				return false;
				});
		}
		ImGui::SameLine();
		if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-right"_h).c_str()))
		{
			add_event([this]() {
				if (folder_history_idx + 1 < folder_history.size())
				{
					folder_history_idx++;
					open_folder(folder_history[folder_history_idx], true);
				}
				return false;
				});
		}
		ImGui::SameLine();
		if (ImGui::Button(graphics::FontAtlas::icon_s("arrow-up"_h).c_str()))
		{
			add_event([this]() {
				if (opened_folder && opened_folder->parent)
					open_folder(opened_folder->parent);
				return false;
				});
		}
		if (opened_folder)
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(Path::reverse(opened_folder->path).string().c_str());
		}

		static std::filesystem::path action_tar;
		static std::string action_str;
		auto open_rename = false;
		auto open_delete_confirm = false;

		ImGui::BeginChild("contents", ImVec2(0, -ImGui::GetFontSize() * 2 - style.ItemSpacing.y * 3));
		auto just_selected = false;
		if (!items.empty())
		{
			auto window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			auto spacing = style.ItemSpacing.x;
			auto item_size = Item::metric.size + Item::metric.padding.x * 2;
			for (auto i = 0; i < items.size(); i++)
			{
				auto& item = items[i];

				ImGui::PushID(i);
				just_selected |= item->draw();
				ImGui::PopID();

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Show In Explorer"))
						exec(L"", std::format(L"explorer /select,\"{}\"", item->path.wstring()));
					if (ImGui::BeginMenu("Copy Path"))
					{
						if (ImGui::MenuItem("Name"))
							set_clipboard(item->path.filename().wstring());
						if (ImGui::MenuItem("Path"))
							set_clipboard(Path::reverse(item->path).wstring());
						if (ImGui::MenuItem("Absolute Path"))
							set_clipboard(item->path.wstring());
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Rename"))
					{
						action_tar = item->path;
						action_str = item->path.filename().string();
						open_rename = true;
					}
					if (ImGui::MenuItem("Delete"))
					{
						action_tar = item->path;
						open_delete_confirm = true;
					}
					ImGui::EndPopup();
				}

				float next_x2 = ImGui::GetItemRectMax().x + spacing + item_size;
				if (i + 1 < items.size() && next_x2 < window_visible_x2)
					ImGui::SameLine();
			}
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !just_selected)
		{
			if (select_callback)
				select_callback(L"");
		}
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
		{
			if (ImGui::MenuItem("Show In Explorer"))
			{
				if (opened_folder)
					exec(L"", std::format(L"explorer /select,\"{}\"", opened_folder->path.wstring()));
			}
			if (ImGui::MenuItem("New Folder"))
			{
				if (opened_folder)
				{
					auto i = 0;
					auto path = opened_folder->path / (L"new_foler_" + wstr(i));
					while (std::filesystem::exists(path))
					{
						i++;
						path = opened_folder->path / (L"new_foler_" + wstr(i));
					}
					std::filesystem::create_directory(path);
				}
			}
			ImGui::EndPopup();
		}
		ImGui::EndChild();

		ImGui::EndTable();

		if (open_rename)
			ImGui::OpenPopup("rename");
		if (ImGui::BeginPopupModal("rename"))
		{
			ImGui::InputText("name", &action_str);
			if (ImGui::Button("OK"))
			{
				auto new_name = action_tar;
				new_name.replace_filename(action_str);
				std::error_code ec;
				std::filesystem::rename(action_tar, new_name, ec);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if (open_delete_confirm)
			ImGui::OpenPopup("delete_confirm");
		if (ImGui::BeginPopupModal("delete_confirm"))
		{
			ImGui::Text("Are you sure to delete \"%s\" ?", action_str.c_str());
			if (ImGui::Button("Yes"))
			{
				std::error_code ec;
				std::filesystem::remove(action_tar, ec);
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("No"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
}
