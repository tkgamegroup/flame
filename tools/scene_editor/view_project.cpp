#include "selection.h"
#include "view_project.h"
#include "view_scene.h"

#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>

View_Project view_project;

View_Project::FolderTreeNode::FolderTreeNode(const std::filesystem::path& path) :
	path(path)
{
	display_text = path.stem().string();
}

void View_Project::FolderTreeNode::read_children()
{
	if (read)
		return;

	children.clear();
	for (auto& it : std::filesystem::directory_iterator(path))
	{
		if (std::filesystem::is_directory(it.status()))
		{
			auto c = new FolderTreeNode(it.path());
			c->parent = this;
			children.emplace_back(c);
		}
	}
	read = true;
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
		auto d = get_thumbnail(metric.size, path);
		auto img = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
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
			if (d.second)
			{
				auto img = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
				thumbnail = img;
				view_project.icons.emplace(icon_id, img);
			}
		}
	}
}

static std::filesystem::path	rename_tar;
static std::string				rename_str;

void View_Project::Item::draw()
{
	ImGui::InvisibleButton("", ImVec2(metric.size + metric.padding.x * 2, metric.size + metric.line_height + metric.padding.y * 3));
	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	auto pressed = ImGui::IsItemClicked();
	auto active = ImGui::IsItemActive();
	auto hovered = ImGui::IsItemHovered();
	ImU32 col;
	if		(active)								col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if (hovered || selection.selecting(path))	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	else											col = ImColor(0, 0, 0, 0);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(p0, p1, col);
	draw_list->AddImage(thumbnail, ImVec2(p0.x + metric.padding.x, p0.y + metric.padding.y), ImVec2(p1.x - metric.padding.x, p1.y - metric.line_height - metric.padding.y * 2));
	draw_list->AddText(ImVec2(p0.x + metric.padding.x + (metric.size - display_text_width) / 2, p0.y + metric.size + metric.padding.y * 2), ImColor(255, 255, 255), display_text.c_str(), display_text.c_str() + display_text.size());

	auto ext = path.extension();

	if (pressed)
		selection.select(path);
	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && active)
	{
		if (std::filesystem::is_directory(path))
		{
			// open folder will destroy all items, so stage path here
			auto p = path;
			add_event([p]() {
				view_project.open_folder(view_project.find_folder(p));
				return false;
			});

			selection.clear();
		}
		else
		{
			if (ext == L".prefab")
				app.open_prefab(path);
		}
	}

	if (ImGui::BeginDragDropSource())
	{
		auto str = path.wstring();
		ImGui::SetDragDropPayload("File", str.c_str(), sizeof(wchar_t) * (str.size() + 1));
		ImGui::TextUnformatted("File");
		ImGui::EndDragDropSource();
	}
}

View_Project::View_Project() :
	View("Project")
{
}

void View_Project::reset()
{
	items.clear();
	opened_folder = nullptr;
	folder_tree.reset(new View_Project::FolderTreeNode(app.project_path));

	if (ev_watcher)
	{
		set_native_event(ev_watcher);
		ev_watcher = nullptr;
	}
	ev_watcher = add_file_watcher(app.project_path, [this](FileChangeFlags flags, const std::filesystem::path& path) {
		mtx_changed_paths.lock();
		auto add_path = [&](const std::filesystem::path& path, FileChangeFlags flags) {
			auto it = changed_paths.find(path);
			if (it == changed_paths.end())
				changed_paths[path] = flags;
			else
				it->second = (FileChangeFlags)(it->second | flags);
		};
		add_path(path.parent_path(), FileModified);
		add_path(path, flags);
		mtx_changed_paths.unlock();
	}, true, false);
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

View_Project::FolderTreeNode* View_Project::find_folder(const std::filesystem::path& path, bool force_read)
{
	std::function<FolderTreeNode*(FolderTreeNode*)> sub_find;
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

void View_Project::open_folder(FolderTreeNode* folder)
{
	opened_folder = folder;
	open_folder_frame = frames;
	if (!folder)
		return;

	folder->read_children();

	if (Item::metric.size == 0)
		set_items_size(64);

	graphics::Queue::get()->wait_idle();

	items.clear();
	icons.clear();
	thumbnails.clear();

	std::vector<Item*> dirs;
	std::vector<Item*> files;
	for (auto& it : std::filesystem::directory_iterator(folder->path))
	{
		if (std::filesystem::is_directory(it.status()))
			dirs.push_back(new Item(it.path()));
		else
			files.push_back(new Item(it.path()));
	}
	std::sort(dirs.begin(), dirs.end(), [](const auto& a, const auto& b) {
		return a->path < b->path;
	});
	std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
		return a->path < b->path;
	});
	for (auto i : dirs)
		items.emplace_back(i);
	for (auto i : files)
		items.emplace_back(i);
}

void View_Project::on_draw()
{
	mtx_changed_paths.lock();
	if (!changed_paths.empty())
	{
		std::vector<std::pair<std::filesystem::path, FileChangeFlags>> changed_directories;
		std::vector<std::pair<std::filesystem::path, FileChangeFlags>> changed_files;
		for (auto& p : changed_paths)
		{
			if (std::filesystem::is_directory(p.first))
				changed_directories.emplace_back(p.first, p.second);
			else
				changed_files.emplace_back(p.first, p.second);
		}
		std::sort(changed_directories.begin(), changed_directories.end(), [](const auto& a, const auto& b) {
			return a.first.wstring().size() < b.first.wstring().size();
		});
		std::sort(changed_files.begin(), changed_files.end(), [](const auto& a, const auto& b) {
			return a.first.wstring().size() < b.first.wstring().size();
		});

		for (auto& p : changed_directories)
		{
			std::function<bool(FolderTreeNode* node)> find_and_mark;
			find_and_mark = [&](FolderTreeNode* node) {
				if (node->path == p.first)
				{
					if (node->read)
					{
						node->read = false;
						node->read_children();
					}
					return true;
				}
				for (auto& c : node->children)
				{
					if (find_and_mark(c.get()))
						return true;
				}
			};
			find_and_mark(folder_tree.get());
		}

		open_folder(opened_folder ? find_folder(opened_folder->path) : nullptr);

		std::vector<std::pair<AssetManagemant::Asset*, std::filesystem::path>> changed_assets;
		for (auto& p : changed_files)
		{
			if (p.second & FileModified || p.second & FileRemoved || p.second & FileRenamed)
			{
				auto asset = AssetManagemant::find(p.first);
				if (asset)
					changed_assets.emplace_back(asset, p.first);
			}
		}
		if (!changed_assets.empty())
		{
			if (app.e_prefab)
			{
				std::vector<std::tuple<void*, Attribute*, std::filesystem::path>> affected_attributes;
				app.e_prefab->forward_traversal([&](EntityPtr e) {
					for (auto& c : e->components)
					{
						auto& ui = *find_udt(c->type_hash);
						for (auto& a : ui.attributes)
						{
							if (a.type == TypeInfo::get<std::filesystem::path>())
							{
								auto value = std::filesystem::path(a.serialize(c.get()));
								auto abs_value = Path::get(value);
								for (auto& asset : changed_assets)
								{
									if (abs_value == asset.second)
									{
										std::filesystem::path p;
										a.set_value(c.get(), &p);
										affected_attributes.emplace_back(c.get(), &a, value);
										break;
									}
								}
							}
						}
					}
				});
				for (auto& a : affected_attributes)
					std::get<1>(a)->set_value(std::get<0>(a), &std::get<2>(a));
			}
		}

		changed_paths.clear();
	}
	mtx_changed_paths.unlock();

	if (!peeding_open_path.empty())
	{
		open_folder(find_folder(peeding_open_path, true));
		peeding_open_path.clear();
	}

	auto just_select = open_folder_frame == (int)frames - 1;

	std::vector<FolderTreeNode*> open_nodes;
	if (just_select && opened_folder)
	{
		auto f = opened_folder->parent;
		while (f)
		{
			open_nodes.push_back(f);
			f = f->parent;
		}
	}

	if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
	{
		auto& style = ImGui::GetStyle();

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("folders", ImVec2(0, -2));
		if (folder_tree)
		{
			std::function<void(FolderTreeNode* node)> draw_node;
			draw_node = [&](FolderTreeNode* node) {
				auto flags = view_project.opened_folder == node ? ImGuiTreeNodeFlags_Selected : 0;
				if (node->read && node->children.empty())
					flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				else
					flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
				if (std::find(open_nodes.begin(), open_nodes.end(), node) != open_nodes.end())
					ImGui::SetNextItemOpen(true);
				auto opened = ImGui::TreeNodeEx(node->display_text.c_str(), flags) && !(flags & ImGuiTreeNodeFlags_Leaf);
				if (just_select && view_project.opened_folder == node)
					ImGui::SetScrollHereY();
				if (ImGui::IsItemClicked())
				{
					if (view_project.opened_folder != node)
						view_project.open_folder(node);
				}
				if (opened)
				{
					node->read_children();
					for (auto& c : node->children)
						draw_node(c.get());
					ImGui::TreePop();
				}
			};

			draw_node(folder_tree.get());
		}
		ImGui::EndChild();

		auto open_rename = false;

		ImGui::TableSetColumnIndex(1);
		if (opened_folder)
			ImGui::TextUnformatted(Path::reverse(opened_folder->path).string().c_str());
		ImGui::BeginChild("contents", ImVec2(0, -ImGui::GetFontSize() * 2 - style.ItemSpacing.y * 3));
		if (!items.empty())
		{
			auto window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			auto spacing = style.ItemSpacing.x;
			auto item_size = Item::metric.size + Item::metric.padding.x * 2;
			for (auto i = 0; i < items.size(); i++)
			{
				auto& item = items[i];

				ImGui::PushID(i);
				item->draw();
				ImGui::PopID();

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Show In Explorer"))
					{

					}
					if (ImGui::BeginMenu("Copy Path"))
					{
						ImGui::MenuItem("Path");
						ImGui::MenuItem("Full Path");
						ImGui::MenuItem("Absolute Path");
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Rename"))
					{
						rename_tar = item->path;
						rename_str = item->path.filename().string();
						open_rename = true;
					}
					if (ImGui::MenuItem("Delete"))
					{

					}
					ImGui::EndPopup();
				}

				float next_x2 = ImGui::GetItemRectMax().x + spacing + item_size;
				if (i + 1 < items.size() && next_x2 < window_visible_x2)
					ImGui::SameLine();
			}
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowFocused() && selection.frame != frames)
			selection.clear();
		ImGui::EndChild();
		
		if (selection.type == Selection::tFile)
			ImGui::TextUnformatted(selection.path.filename().string().c_str());

		ImGui::EndTable();

		if (open_rename)
			ImGui::OpenPopup("rename");
		if (ImGui::BeginPopupModal("rename"))
		{
			ImGui::InputText("name", &rename_str);
			if (ImGui::Button("OK"))
			{
				auto new_name = rename_tar;
				new_name.replace_filename(rename_str);
				std::error_code ec;
				std::filesystem::rename(rename_tar, new_name, ec);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
}
