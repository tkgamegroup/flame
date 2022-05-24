#include "selection.h"
#include "view_project.h"
#include "view_scene.h"

#include <flame/foundation/bitmap.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/model.h>
#include <flame/graphics/shader.h>

View_Project view_project;

View_Project::View_Project() :
	View("Project")
{
}

void View_Project::reset(const std::filesystem::path& assets_path)
{
	explorer.reset(assets_path);

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
	explorer.select_callback = [this](const std::filesystem::path& path) {
		if (path.empty())
			selection.clear();
		else
			selection.select(path);
	};
	explorer.dbclick_callback = [this](const std::filesystem::path& path) {
		auto ext = path.extension();
		if (ext == L".prefab")
			app.open_prefab(path);
	};
	explorer.item_context_menu_callback = [this](const std::filesystem::path& path) {
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
			ImGui::OpenInputDialog("New name", [path](bool ok, const std::string& text) {
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
			ImGui::OpenYesNoDialog(std::format("Are you sure to delete \"{}\" ?", path.string()), [path](bool yes) {
				if (yes)
				{
					std::error_code ec;
					std::filesystem::remove(path, ec);
				}
			});
		}
	};
	explorer.folder_context_menu_callback = [this](const std::filesystem::path& path) {
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
		if (ImGui::MenuItem("New Image"))
		{
			struct NewImageDialog : ImGui::Dialog
			{
				std::filesystem::path dir;
				std::string name = "new_image";
				int format = 0;
				ivec2 size = ivec2(256);
				int type = 0;
				vec2 noise_offset = vec2(3.8f, 7.5f);
				float noise_scale = 4.f;
				float noise_falloff = 10.f;
				float noise_power = 3.f;

				std::unique_ptr<graphics::Image> image;

				static void open(const std::filesystem::path& dir)
				{
					auto dialog = new NewImageDialog;
					dialog->title = "New Image";
					dialog->dir = dir;
					Dialog::open(dialog);
				}

				graphics::ImagePtr generate_image()
				{
					if (size.x <= 0 || size.y <= 0)
						return nullptr;

					graphics::Format fmt;
					switch (format)
					{
					case 0: fmt = graphics::Format_R8G8B8A8_UNORM; break;
					case 1: fmt = graphics::Format_R8_UNORM; break;
					}

					auto ret = graphics::Image::create(fmt, (uvec2)size, graphics::ImageUsageTransferSrc | graphics::ImageUsageAttachment | graphics::ImageUsageSampled);

					{
						graphics::InstanceCB cb;
						cb->image_barrier(ret, {}, graphics::ImageLayoutAttachment);
						cb->set_viewport_and_scissor(Rect(vec2(0), vec2(size)));
						switch (type)
						{
						case 0:
							cb->begin_renderpass(nullptr, ret->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(0.f) });
							cb->end_renderpass();
							break;
						case 1:
							cb->begin_renderpass(nullptr, ret->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(1.f) });
							cb->end_renderpass();
							break;
						case 2:
						{
							auto fb = ret->get_shader_write_dst();
							auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\noise\\fbm.pipeline",
								{ "rp=" + str(fb->renderpass) });
							graphics::PipelineResourceManager<FLAME_UID> prm;
							prm.init(pl->layout);

							cb->begin_renderpass(nullptr, fb);
							cb->bind_pipeline(pl);
							prm.set_pc_var<"uv_off"_h>(noise_offset);
							prm.set_pc_var<"uv_scl"_h>(noise_scale);
							prm.set_pc_var<"val_base"_h>(0.f);
							prm.set_pc_var<"val_scl"_h>(1.f);
							prm.set_pc_var<"falloff"_h>(1.f / clamp(noise_falloff, 2.f, 100.f));
							prm.set_pc_var<"power"_h>(noise_power);
							prm.push_constant(cb.get());
							cb->draw(3, 1, 0, 0);
							cb->end_renderpass();
						}
							break;
						}
						cb->image_barrier(ret, {}, graphics::ImageLayoutShaderReadOnly);
					}

					return ret;
				}

				void draw() override
				{
					bool open = true;
					if (ImGui::Begin(title.c_str(), &open))
					{
						ImGui::InputText("name", &name);
						static const char* formats[] = {
							"RGBA8",
							"R8"
						};
						ImGui::Combo("format", &format, formats, countof(formats));
						ImGui::InputInt2("size", &size[0]);
						static const char* types[] = {
							"Black",
							"White",
							"Fbm"
						};
						ImGui::Combo("type", &type, types, countof(types));
						switch (type)
						{
						case 2:
							ImGui::DragFloat2("offset", (float*)&noise_offset, 0.1f, 0.f, 100.f);
							ImGui::DragFloat("scale", &noise_scale, 0.1f, 0.f, 10.f);
							ImGui::DragFloat("falloff", &noise_falloff, 1.f, 2.f, 100.f);
							ImGui::DragFloat("power", &noise_power, 0.01f, 1.f, 10.f);
							break;
						}
						if (ImGui::Button("Generate"))
						{
							graphics::Queue::get()->wait_idle();
							image.reset(generate_image());
						}
						if (image)
							ImGui::Image(image.get(), (vec2)size);
						if (ImGui::Button("Save"))
						{
							if (image && !name.empty())
								image->save(dir / name);
						}
						ImGui::SameLine();
						if (ImGui::Button("Close"))
							close();

						ImGui::End();
					}
					if (!open)
						close();
				}
			};

			NewImageDialog::open(path);
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

			if (p.second != FileModified)
			{
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
		}
		std::sort(changed_directories.begin(), changed_directories.end(), [](const auto& a, const auto& b) {
			return a.wstring().size() < b.wstring().size();
		});
		std::sort(changed_files.begin(), changed_files.end(), [](const auto& a, const auto& b) {
			return a.first.wstring().size() < b.first.wstring().size();
		});

		auto current_path = explorer.opened_folder ? explorer.opened_folder->path : L"";

		for (auto& p : changed_directories)
		{
			if (auto node = explorer.find_folder(p); node && node->read)
			{
				node->read = false;
				node->read_children();
			}
		}

		explorer.open_folder(current_path.empty() ? nullptr : explorer.find_folder(current_path));

		std::vector<std::pair<AssetManagemant::Asset*, std::filesystem::path>> changed_assets;
		for (auto& p : changed_files)
		{
			if ((p.second & FileModified) || (p.second & FileRemoved) || (p.second & FileRenamed))
			{
				auto asset = AssetManagemant::find(p.first);
				if (asset && asset->active)
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
			explorer.ping(selection.path());
		explorer.selected_path = selection.path();
	}
	else
		explorer.selected_path.clear();

	explorer.draw();
}
