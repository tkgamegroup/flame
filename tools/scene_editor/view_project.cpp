#include "selection.h"
#include "view_project.h"
#include "view_scene.h"

#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/model.h>

View_Project view_project;

View_Project::FolderTreeNode::FolderTreeNode(const std::filesystem::path& path) :
	path(path)
{
	display_text = path.filename().string();
}

void View_Project::FolderTreeNode::read_children()
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
				auto c = new FolderTreeNode(it.path());
				c->parent = this;
				children.emplace_back(c);
			}
		}
	}
	read = true;
}

View_Project::Item::Metric View_Project::Item::metric = {};

View_Project::Item::Item(const std::filesystem::path& path, const std::string& text, graphics::ImagePtr image) :
	path(path),
	text(text),
	image(image)
{
	prune_text();
}

View_Project::Item::Item(const std::filesystem::path& path) :
	path(path)
{
	text = path.filename().string();
	prune_text();

	auto ext = path.extension();
	if (ext == L".fmod")
		image = view_project.icons[Icon_Model];
	else if (is_image_file(ext))
	{
		auto d = get_thumbnail(metric.size, path);
		auto img = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
		image = img;
		view_project.thumbnails.emplace_back(img);
	}
	else
	{
		int id;
		get_icon(path.c_str(), &id);
		auto it = view_project.sys_icons.find(id);
		if (it != view_project.sys_icons.end())
			image = it->second.get();
		else
		{
			auto d = get_icon(path.c_str(), nullptr);
			if (d.second)
			{
				auto img = graphics::Image::create(graphics::Format_B8G8R8A8_UNORM, d.first, d.second.get());
				image = img;
				view_project.sys_icons.emplace(id, img);
			}
		}
	}
}

void View_Project::Item::prune_text()
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

void View_Project::Item::draw()
{
	ImGui::InvisibleButton("", ImVec2(metric.size + metric.padding.x * 2, metric.size + metric.line_height + metric.padding.y * 3));
	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	auto hovered = ImGui::IsItemHovered();
	auto active = ImGui::IsItemActive();
	ImU32 col;
	if		(active)								col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if (hovered || selection.selecting(path))	col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	else											col = ImColor(0, 0, 0, 0);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(p0, p1, col);
	draw_list->AddImage(image, ImVec2(p0.x + metric.padding.x, p0.y + metric.padding.y), ImVec2(p1.x - metric.padding.x, p1.y - metric.line_height - metric.padding.y * 2));
	draw_list->AddText(ImVec2(p0.x + metric.padding.x + (metric.size - text_width) / 2, p0.y + metric.size + metric.padding.y * 2), ImColor(255, 255, 255), text.c_str(), text.c_str() + text.size());

	auto ext = path.extension();

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hovered && ImGui::IsItemDeactivated())
		selection.select(path);
	if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && active)
	{
		if (has_children)
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

	if (hovered) 
		ImGui::SetTooltip("%s", path.filename().string().c_str());
}

View_Project::View_Project() :
	View("Project")
{
}

void View_Project::reset(const std::filesystem::path& assets_path)
{
	items.clear();
	opened_folder = nullptr;
	folder_tree.reset(new View_Project::FolderTreeNode(assets_path));

	if (ev_watcher)
	{
		set_native_event(ev_watcher);
		ev_watcher = nullptr;
	}
	ev_watcher = add_file_watcher(assets_path, [this](FileChangeFlags flags, const std::filesystem::path& path) {
		mtx_changed_paths.lock();
		auto it = changed_paths.find(path);
		if (it == changed_paths.end())
			changed_paths[path] = flags;
		else
			it->second = (FileChangeFlags)(it->second | flags);
		mtx_changed_paths.unlock();
	}, true, false);
}

void View_Project::init()
{
	Item::metric.size = 64;
	Item::metric.padding = ImGui::GetStyle().FramePadding;
	Item::metric.line_height = ImGui::GetTextLineHeight();

	auto curr_path = std::filesystem::current_path();
	icons[Icon_Model] = graphics::Image::get(curr_path / L"icon_model.png");
	icons[Icon_Armature] = graphics::Image::get(curr_path / L"icon_armature.png");
	icons[Icon_Mesh] = graphics::Image::get(curr_path / L"icon_mesh.png");
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

void View_Project::open_folder(FolderTreeNode* folder, bool from_histroy)
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

	folder->read_children();

	items.clear();

	graphics::Queue::get()->wait_idle();
	thumbnails.clear();

	if (std::filesystem::is_directory(folder->path))
	{
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
					items.emplace_back(new Item(path, "armature", icons[Icon_Armature]));
				}
				for (auto i = 0; i < model->meshes.size(); i++)
				{
					auto path = folder->path;
					path += L":" + wstr(i);
					items.emplace_back(new Item(path, str(i), icons[Icon_Mesh]));
				}
			}
		}
	}
}

void View_Project::on_draw()
{
	mtx_changed_paths.lock();
	if (!changed_paths.empty())
	{
		std::vector<std::filesystem::path> changed_directories;
		std::vector<std::pair<std::filesystem::path, FileChangeFlags>> changed_files;
		for (auto& p : changed_paths)
		{
			if (std::filesystem::is_directory(p.first) && p.second == FileModified)
				changed_directories.push_back(p.first);
			else
				changed_files.emplace_back(p.first, p.second);

			auto parent_path = p.first.parent_path();
			auto found = false;
			for (auto& pp : changed_directories)
			{
				if (pp == parent_path)
				{
					found = true;
					break;
				}
			}
			if (!found)
				changed_directories.push_back(parent_path);
		}
		std::sort(changed_directories.begin(), changed_directories.end(), [](const auto& a, const auto& b) {
			return a.wstring().size() < b.wstring().size();
		});
		std::sort(changed_files.begin(), changed_files.end(), [](const auto& a, const auto& b) {
			return a.first.wstring().size() < b.first.wstring().size();
		});

		for (auto& p : changed_directories)
		{
			if (auto node = find_folder(p); node && node->read)
			{
				node->read = false;
				node->read_children();
			}
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
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && selection.frame != frames)
			selection.clear();
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
		
		if (selection.type == Selection::tPath)
			ImGui::TextUnformatted(selection.path().filename().string().c_str());

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
