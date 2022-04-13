#include "selection.h"
#include "view_project.h"
#include "view_scene.h"
#include "dialog.h"

#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/model.h>

View_Project view_project;

View_Project::View_Project() :
	View("Project")
{
}

void View_Project::reset(const std::filesystem::path& assets_path)
{
	resource_panel.reset(assets_path);

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
	resource_panel.select_callback = [this](const std::filesystem::path& path) {
		if (path.empty())
			selection.clear();
		else
			selection.select(path);
	};
	resource_panel.dbclick_callback = [this](const std::filesystem::path& path) {
		auto ext = path.extension();
		if (ext == L".prefab")
			app.open_prefab(path);
	};
	resource_panel.item_context_menu_callback = [this](const std::filesystem::path& path) {
		if (ImGui::MenuItem("Show In Explorer"))
			exec(L"", std::format(L"explorer /select,\"{}\"", path.wstring()));
		if (ImGui::BeginMenu("Copy Path"))
		{
			if (ImGui::MenuItem("Name"))
				set_clipboard(path.filename().wstring());
			if (ImGui::MenuItem("Path"))
				set_clipboard(Path::reverse(path).wstring());
			if (ImGui::MenuItem("Absolute Path"))
				set_clipboard(path.wstring());
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Rename"))
		{
			InputDialog::open("New name", [path](bool ok, const std::string& text) {
				if (ok)
				{
					auto new_name = path;
					new_name.replace_filename(text);
					std::error_code ec;
					std::filesystem::rename(path, new_name, ec);
				}
			});
		}
		if (ImGui::MenuItem("Delete"))
		{
			YesNoDialog::open(std::format("Are you sure to delete \"{}\" ?", path.string()), [path](bool yes) {
				if (yes)
				{
					std::error_code ec;
					std::filesystem::remove(path, ec);
				}
			});
		}
	};
	resource_panel.folder_context_menu_callback = [this](const std::filesystem::path& path) {
		if (ImGui::MenuItem("Show In Explorer"))
			exec(L"", std::format(L"explorer /select,\"{}\"", path.wstring()));
		if (ImGui::MenuItem("New Folder"))
		{
			auto i = 0;
			auto p = path / (L"new_foler_" + wstr(i));
			while (std::filesystem::exists(p))
			{
				i++;
				p = path / (L"new_foler_" + wstr(i));
			}
			std::filesystem::create_directory(p);
		}
	};
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
			if (auto node = resource_panel.find_folder(p); node && node->read)
			{
				node->read = false;
				node->read_children();
			}
		}

		resource_panel.open_folder(resource_panel.opened_folder ? 
			resource_panel.find_folder(resource_panel.opened_folder->path) : nullptr);

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

	if (selection.type == Selection::tPath)
	{
		if (selection.frame == (int)frames - 1)
			resource_panel.ping(selection.path());
		resource_panel.selected_path = selection.path();
	}
	else
		resource_panel.selected_path.clear();

	resource_panel.draw();
}
