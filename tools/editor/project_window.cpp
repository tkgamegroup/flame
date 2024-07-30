#include "selection.h"
#include "project_window.h"
#include "scene_window.h"
#include "blueprint_window.h"
#include "sheet_window.h"

#include <flame/foundation/bitmap.h>
#include <flame/foundation/system.h>
#include <flame/foundation/blueprint.h>
#include <flame/foundation/sheet.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/material.h>
#include <flame/graphics/model.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/extension.h>
#include <flame/graphics/debug.h>
#include <flame/universe/timeline.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>

static graphics::ImagePtr icon_prefab;
static graphics::ImagePtr icon_material;
static graphics::ImagePtr icon_mesh;
static graphics::ImagePtr icon_armature;

static std::deque<std::filesystem::path> recent_paths;

static std::filesystem::path get_unique_filename(const std::filesystem::path& prefix, const std::filesystem::path& ext = L"")
{
	auto i = 0;
	auto p = prefix;
	p += wstr(i);
	p += ext;
	while (std::filesystem::exists(p))
	{
		i++;
		p = prefix;
		p += wstr(i);
		p += ext;
	}
	return p;
}

static void update_thumbnail(const std::filesystem::path& path)
{
	auto ext = path.extension();
	if (ext == L".prefab")
	{
		static std::vector<std::filesystem::path> paths;
		static void* ev_process = nullptr;
		if (std::find(paths.begin(), paths.end(), path) != paths.end())
			return;
		paths.push_back(path);

		static ModelPreviewer previewer;

		if (!ev_process)
		{
			ev_process = add_event([]() {
				if (paths.empty())
				{
					ev_process = nullptr;
					return false;
				}
				if (previewer.image) // if updating then return
					return true;

				auto path = paths.back();
				paths.pop_back();

				previewer.init();
				auto e = Entity::create();
				e->load(path);
				e->forward_traversal([](EntityPtr e) {
					e->layer = previewer.layer;
				});
				previewer.model->add_child(e);
				previewer.update(frames, false);

				add_event([path]() {
					graphics::Queue::get()->wait_idle();

					auto thumbnails_dir = path.parent_path() / L".thumbnails";
					if (!std::filesystem::exists(thumbnails_dir))
						std::filesystem::create_directories(thumbnails_dir);
					auto thumbnail_path = thumbnails_dir / (path.filename().wstring() + L".png");
					previewer.image->save(thumbnail_path);

					for (auto& v : project_window.views)
					{
						auto pv = (ProjectView*)v.get();
						for (auto& item : pv->explorer.items)
						{
							if (item->path == path)
							{
								if (!item->icon_releaser)
								{
									if (item->icon)
										graphics::release_icon(item->icon);
								}
								else
									item->icon_releaser(item->icon);

								item->icon = graphics::Image::get(thumbnail_path);
								item->icon_releaser = [](graphics::Image* img) {
									graphics::Image::release(img);
								};

								break;
							}
						}
					}

					previewer.destroy();

					return false;
				}, 0.f, 3);

				app.render_frames += 3;

				return true;
			});
		}
	}
}

ProjectWindow project_window;
static auto selection_changed = false;

ProjectView::ProjectView() :
	ProjectView(project_window.views.empty() ? "Project" : "Project##" + str(rand()))
{
}

ProjectView::ProjectView(const std::string& name) :
	View(&project_window, name)
{
	explorer.select_button = ImGuiMouseButton_Middle;
	explorer.select_callback = [this](const std::filesystem::path& path) {
		if (path.empty())
		{
			selection.clear("project"_h);
			return;
		}
		if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl))
		{
			auto paths = selection.get_paths();
			auto found = false;
			for (auto it = paths.begin(); it != paths.end();)
			{
				if (*it == path)
				{
					found = true;
					it = paths.erase(it);
					break;
				}
				else
					it++;
			}
			if (!found)
				paths.push_back(path);
			selection.select(paths);
		}
		else
			selection.select(path, "project"_h);

		explorer.selected_paths = selection.get_paths();
	};
	explorer.dbclick_callback = [this](const std::filesystem::path& path) {
		auto ext = path.extension();
		if (ext == L".bp")
		{
			auto opened = false;
			for (auto& v : blueprint_window.views)
			{
				auto bv = (BlueprintView*)v.get();
				if (Path::get(bv->blueprint_path) == path)
				{
					if (bv->imgui_window)
						ImGui::FocusWindow((ImGuiWindow*)bv->imgui_window);
					opened = true;
					break;
				}
			}
			if (!opened)
				blueprint_window.open_view(Path::reverse(path).string() + "##Blueprint");
		}
		else if (ext == L".sht")
		{
			auto opened = false;
			for (auto& v : sheet_window.views)
			{
				auto sv = (SheetView*)v.get();
				if (Path::get(sv->sheet_path) == path)
				{
					if (sv->imgui_window)
						ImGui::FocusWindow((ImGuiWindow*)sv->imgui_window);
					opened = true;
					break;
				}
			}
			if (!opened)
				sheet_window.open_view(Path::reverse(path).string() + "##Sheet");
		}
		else if (ext == L".prefab")
		{
			app.open_prefab(path);
			auto sv = scene_window.first_view();
			if (sv && sv->imgui_window)
				ImGui::FocusWindow((ImGuiWindow*)sv->imgui_window);
		}
		else if (ext == L".h" || ext == L".cpp")
			app.open_file_in_vs(path);
		else if (ext == L".timeline")
			app.open_timeline(path);

		auto it = std::find(recent_paths.begin(), recent_paths.end(), path);
		if (it == recent_paths.end())
		{
			recent_paths.push_front(path);
			if (recent_paths.size() > 20)
				recent_paths.pop_back();
		}
		else
			std::rotate(recent_paths.begin(), it, recent_paths.end());
	};
	explorer.item_context_menu_callback = [this](const std::filesystem::path& path) {
		// right click will also select the file
		auto paths = selection.get_paths();
		if (std::find(paths.begin(), paths.end(), path) == paths.end())
		{
			if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Shift) || ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl))
			{
				paths.push_back(path);
				selection.select(paths, "project"_h);
			}
			else
				selection.select(path, "project"_h);
		}

		auto ext = path.extension();

		if (ImGui::MenuItem("Show In Explorer"))
		{
			for (auto& p : paths)
				exec(L"", std::format(L"explorer /select,\"{}\"", p.wstring()));
		}
		if (paths.size() == 1 && is_text_file(path.extension()))
		{
			if (ImGui::MenuItem("Open In VS"))
				app.open_file_in_vs(path);
		}
		if (paths.size() == 1 && ImGui::BeginMenu("Copy Path"))
		{
			if (ImGui::MenuItem("Name"))
				set_clipboard(path.filename().wstring());
			if (ImGui::MenuItem("Path"))
				set_clipboard(Path::reverse(path).wstring());
			if (ImGui::MenuItem("Absolute Path"))
				set_clipboard(path.wstring());
			ImGui::EndMenu();
		}
		{
			auto& favorites = app.project_settings.favorites;
			auto it_favorites = std::find(favorites.begin(), favorites.end(), path);
			if (it_favorites == favorites.end())
			{
				if (ImGui::MenuItem("Add to Favorites (Shift+F4)"))
					favorites.push_back(path);
			}
			else
			{
				if (ImGui::MenuItem("Remove from Favorites (Shift+F4)"))
					favorites.erase(it_favorites);
			}
		}
		if (ImGui::MenuItem("Copy"))
		{
			set_clipboard_files(paths);
		}
		if (paths.size() == 1 && ImGui::MenuItem("Rename"))
		{
			explorer.enter_rename(path);
			explorer.rename_start_frame++;
		}
		if (ImGui::MenuItem("Delete"))
		{
			std::string names;
			for (auto& p : paths)
				names += p.string() + "\n";
			ImGui::OpenYesNoDialog("Delete asset(s)?", names, [paths](bool yes) {
				if (yes)
				{
					for (auto& p : paths)
					{
						std::error_code ec;
						std::filesystem::remove(p, ec);
					}
				}
				});
		}
		if (ext == L".h" || ext == L".cpp")
		{
			ImGui::Separator();
			if (ImGui::MenuItem("Edit Interfaces"))
			{
				struct EditInterfacesDialog : ImGui::Dialog
				{
					std::filesystem::path header_path;
					std::filesystem::path source_path;
					bool invalid = false;

					std::vector<std::pair<std::string, bool>> overrides;
					std::vector<std::string> attributes;
					std::vector<std::string> functions;

					static void open(const std::filesystem::path& path)
					{
						auto dialog = new EditInterfacesDialog;
						dialog->title = "Edit Interfaces";
						dialog->init(path);
						Dialog::open(dialog);
					}

					void init(const std::filesystem::path& path)
					{
						header_path = path;
						header_path.replace_extension(L".h");
						source_path = path;
						source_path.replace_extension(L".cpp");

						if (!std::filesystem::exists(header_path) || !std::filesystem::exists(source_path))
						{
							invalid = true;
							return;
						}

						for (auto& fi : TypeInfo::get<Component>()->retrive_ui()->functions)
							overrides.emplace_back(fi.name, false);
					}

					void draw() override
					{
						bool open = true;
						if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_NoSavedSettings))
						{
							if (!invalid)
							{
								ImGui::TextUnformatted("Overrides");
								for (auto& o : overrides)
									ImGui::Checkbox(o.first.c_str(), &o.second);
								ImGui::Separator();
								ImGui::TextUnformatted("Attributes");
								ImGui::Button("Add##2");
								ImGui::Separator();
								ImGui::TextUnformatted("Functions");
								ImGui::Button("Add##3");
								ImGui::Separator();

								if (ImGui::Button("Confirm"))
									;
							}
							else
							{
								ImGui::TextUnformatted("Code file are invalid");
								if (ImGui::Button("Refresh"))
									;
							}

							ImGui::End();
						}
						if (!open)
							close();
					}
				};
			}
		}
		else if (ext == L".timeline")
		{
			if (ImGui::MenuItem("Open"))
				app.open_timeline(path);
		}
		if (ImGui::MenuItem("Refresh"))
			update_thumbnail(path);
		if (ImGui::BeginMenu("Tools"))
		{
			if (is_image_file(ext))
			{
				if (ImGui::MenuItem("Create Material Use This Image"))
				{
					std::filesystem::path dst_dir;
					if (path.parent_path().stem() == L"textures")
						dst_dir += L"../materials";
					else
						dst_dir = path.parent_path();
					if (!std::filesystem::exists(dst_dir))
						std::filesystem::create_directories(dst_dir);

					for (auto& p : paths)
					{
						if (is_image_file(p.extension()))
						{
							auto material = graphics::Material::create();
							material->textures.resize(1);
							material->textures[0].filename = Path::reverse(p);
							material->color_map = 0;
							auto fn = dst_dir / (p.stem().wstring() + L".fmat");
							material->save(fn);
							delete material;
						}
					}
				}
			}
			else if (ext == L".fmat")
			{
				if (ImGui::MenuItem("Assign This Material To All Selected Prefabs"))
				{
					for (auto& p : paths)
					{
						if (p.extension() == L".prefab")
						{
							auto e = Entity::create();
							e->load(p);
							e->forward_traversal([&](EntityPtr e) {
								if (auto mesh = e->get_component<cMesh>(); mesh)
									mesh->material_name = Path::reverse(path);
							});
							e->save(p);
							delete e;
						}
					}
				}
			}
			ImGui::EndMenu();
		}

		explorer.selected_paths = selection.get_paths();
	};
	explorer.folder_context_menu_callback = [this](const std::filesystem::path& path) {
		auto in_assets = false;
		auto in_cpp = false;
		for (auto it = path.begin(); it != path.end(); it++)
		{
			if (*it == L"assets")
			{
				in_assets = true;
				break;
			}
			if (*it == L"cpp")
			{
				in_cpp = true;
				break;
			}
		}

		if (ImGui::MenuItem("Show In Explorer"))
			exec(L"", std::format(L"explorer /select,\"{}\"", path.wstring()));
		if (!get_clipboard_files(true).empty())
		{
			if (ImGui::MenuItem("Paste"))
			{
				for (auto& file : get_clipboard_files())
				{
					std::error_code ec;
					std::filesystem::copy_file(file, get_unique_filename(path / file.filename().stem(), file.extension()), ec);
				}
			}
		}
		if (ImGui::MenuItem("New Folder"))
		{
			ImGui::OpenInputDialog("New Folder", "Name", [path](bool ok, const std::string& str) {
				if (ok && !str.empty())
				{
					auto fn = path / str;
					if (!std::filesystem::exists(fn))
						std::filesystem::create_directory(fn);
					else
						ImGui::OpenMessageDialog("Failed to create folder", "Folder already existed");
				}
			});
		}
		if (in_assets)
		{
			if (ImGui::MenuItem("New Blueprint"))
			{
				ImGui::OpenInputDialog("New Blueprint", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".bp");
						if (!std::filesystem::exists(fn))
						{
							auto bp = Blueprint::create();
							bp->save(fn);
						}
						else
							ImGui::OpenMessageDialog("Failed to create Blueprint", "Blueprint already existed");
					}
				});
			}
			if (ImGui::MenuItem("New Sheet"))
			{
				ImGui::OpenInputDialog("New Sheet", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".sht");
						if (!std::filesystem::exists(fn))
						{
							auto sht = Sheet::create();
							sht->save(fn);
						}
						else
							ImGui::OpenMessageDialog("Failed to create Sheet", "Sheet already existed");
					}
				});
			}
			if (ImGui::MenuItem("New Image"))
			{
				struct NewImageDialog : ImGui::Dialog
				{
					enum Format
					{
						Format_RGBA8,
						Format_R8
					};

					enum Type
					{
						TypeBlack,
						TypeWhite,
						TypePerlinNoise,
						TypeSphere,
						TypeColorPalette
					};

					std::filesystem::path dir;
					std::string name = "new_image";
					int format = 0;
					ivec3 extent = ivec3(256, 256, 1);
					int type = 0;

					vec2 noise_offset = vec2(3.8f, 7.5f);
					float noise_scale = 4.f;
					float noise_falloff = 10.f;
					float noise_power = 3.f;

					float color_palette_saturation = 0.5f;
					int color_palette_hud_num = 12;
					int color_palette_value_num = 10;

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
						if (extent.x <= 0 || extent.y <= 0 || extent.z <= 0)
							return nullptr;

						graphics::Format fmt = graphics::Format_Undefined;
						switch (format)
						{
						case Format_RGBA8: fmt = graphics::Format_R8G8B8A8_UNORM; break;
						case Format_R8: fmt = graphics::Format_R8_UNORM; break;
						}

						auto ret = graphics::Image::create(fmt, extent, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageAttachment | graphics::ImageUsageSampled);

						switch (type)
						{
						case TypeBlack:
							ret->clear(vec4(0.f), graphics::ImageLayoutShaderReadOnly);
							break;
						case TypeWhite:
							ret->clear(vec4(1.f), graphics::ImageLayoutShaderReadOnly);
							break;
						case TypePerlinNoise:
							if (extent.z == 1)
							{
								graphics::InstanceCommandBuffer cb;
								cb->image_barrier(ret, {}, graphics::ImageLayoutAttachment);
								cb->set_viewport_and_scissor(Rect(vec2(0), vec2(extent)));

								auto fb = ret->get_shader_write_dst();
								auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\noise\\perlin.pipeline",
									{ "rp=" + str(fb->renderpass) });
								graphics::PipelineResourceManager prm;
								prm.init(pl->layout, graphics::PipelineGraphics);

								cb->begin_renderpass(nullptr, fb);
								cb->bind_pipeline(pl);
								prm.pc.child("uv_off"_h).as<vec2>() = noise_offset;
								prm.pc.child("uv_scl"_h).as<float>() = noise_scale;
								prm.pc.child("val_base"_h).as<float>() = 0.f;
								prm.pc.child("val_scl"_h).as<float>() = 1.f;
								prm.pc.child("falloff"_h).as<float>() = 1.f / clamp(noise_falloff, 2.f, 100.f);
								prm.pc.child("power"_h).as<float>() = noise_power;
								prm.push_constant(cb.get());
								cb->draw(3, 1, 0, 0);
								cb->end_renderpass();
								cb->image_barrier(ret, {}, graphics::ImageLayoutShaderReadOnly);
								cb.excute();
							}
							break;
						case TypeSphere:
							if (extent.z > 1)
							{
								graphics::StagingBuffer sb(ret->data_size, nullptr);
								graphics::InstanceCommandBuffer cb;
								auto dst = (char*)sb->mapped;
								for (auto z = 0; z < extent.z; z++)
								{
									auto zoff = z * extent.x * extent.y;
									auto fz = (((float)z + 0.5f) / extent.z) * 2.f - 1.f;
									for (auto y = 0; y < extent.y; y++)
									{
										auto yoff = y * extent.x;
										auto fy = (((float)y + 0.5f) / extent.y) * 2.f - 1.f;
										for (auto x = 0; x < extent.x; x++)
										{
											auto fx = (((float)x + 0.5f) / extent.x) * 2.f - 1.f;
											dst[x + yoff + zoff] = (1.f - min(1.f, sqrt(fx * fx + fy * fy + fz * fz))) * 255.f;
										}
									}
								}
								cb->image_barrier(ret, {}, graphics::ImageLayoutTransferDst);
								graphics::BufferImageCopy cpy;
								cpy.img_ext = extent;
								cb->copy_buffer_to_image(sb.get(), ret, cpy);
								cb->image_barrier(ret, {}, graphics::ImageLayoutShaderReadOnly);
								cb.excute();
							}
							break;
						case TypeColorPalette:
							if (extent.z == 1)
							{
								graphics::StagingBuffer sb(ret->data_size, nullptr);
								graphics::InstanceCommandBuffer cb;
								auto dst = (char*)sb->mapped;
								for (auto x = 0; x < color_palette_hud_num; x++)
								{
									auto h = (float)x / color_palette_hud_num * 360.f;
									for (auto y = 0; y <= color_palette_value_num; y++)
									{
										auto v = (float)y / color_palette_value_num;
										auto color = rgbColor(vec3(h, color_palette_saturation, v));
										auto idx = (color_palette_value_num - y) * extent.x * 4 + x * 4;
										dst[idx + 0] = (char)(color.x * 255.f);
										dst[idx + 1] = (char)(color.y * 255.f);
										dst[idx + 2] = (char)(color.z * 255.f);
										dst[idx + 3] = 255;
									}
								}
								for (auto y = 0; y <= color_palette_value_num; y++)
								{
									auto v = (float)y / color_palette_value_num;
									auto color = vec3(v);
									auto idx = (color_palette_value_num - y) * extent.x * 4 + color_palette_hud_num * 4;
									dst[idx + 0] = (char)(color.x * 255.f);
									dst[idx + 1] = (char)(color.y * 255.f);
									dst[idx + 2] = (char)(color.z * 255.f);
									dst[idx + 3] = 255;
								}
								cb->image_barrier(ret, {}, graphics::ImageLayoutTransferDst);
								graphics::BufferImageCopy cpy;
								cpy.img_ext = extent;
								cb->copy_buffer_to_image(sb.get(), ret, cpy);
								cb->image_barrier(ret, {}, graphics::ImageLayoutShaderReadOnly);
								cb.excute();
							}
							break;
						}

						return ret;
					}

					void draw() override
					{
						bool open = true;
						if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_NoSavedSettings))
						{
							ImGui::InputText("name", &name);
							static const char* formats[] = {
								"RGBA8",
								"R8"
							};
							ImGui::Combo("format", &format, formats, countof(formats));
							ImGui::InputInt3("size", &extent[0]);
							static const char* types[] = {
								"Black",
								"White",
								"Perlin Noise",
								"Sphere",
								"Color Palette"
							};
							ImGui::Combo("type", &type, types, countof(types));
							switch (type)
							{
							case TypePerlinNoise:
								ImGui::DragFloat2("offset", (float*)&noise_offset, 0.1f, 0.f, 100.f);
								ImGui::DragFloat("scale", &noise_scale, 0.1f, 0.f, 10.f);
								ImGui::DragFloat("falloff", &noise_falloff, 1.f, 2.f, 100.f);
								ImGui::DragFloat("power", &noise_power, 0.01f, 1.f, 10.f);
								break;
							case TypeColorPalette:
								ImGui::DragFloat("saturation", &color_palette_saturation, 0.01f, 0.f, 1.f);
								ImGui::InputInt("hud num", &color_palette_hud_num);
								ImGui::InputInt("value num", &color_palette_value_num);
								break;
							}
							if (ImGui::Button("Generate"))
							{
								graphics::Queue::get()->wait_idle();
								image.reset(generate_image());
							}
							if (image && extent.z == 1)
								ImGui::Image(image.get(), (vec2)extent);
							static bool compress = false;
							ImGui::Checkbox("Compress", &compress);
							if (ImGui::Button("Save"))
							{
								if (image && !name.empty())
									image->save(dir / name, compress);
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
			if (ImGui::MenuItem("New Material"))
			{
				ImGui::OpenInputDialog("New Material", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".fmat");
						if (!std::filesystem::exists(fn))
						{
							auto material = graphics::Material::create();
							material->save(fn);
							delete material;
						}
						else
							ImGui::OpenMessageDialog("Failed to create Material", "Material already existed");
					}
				});
			}
			if (ImGui::MenuItem("New BP Material"))
			{
				ImGui::OpenInputDialog("New BP Material", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".bp");
						if (!std::filesystem::exists(fn))
						{
							auto bp = Blueprint::get(fn);
							auto g = bp->groups.front().get();
							bp->add_group_input(g, "i_mat_id", TypeInfo::get<uint>());
							bp->add_group_input(g, "i_uv", TypeInfo::get<vec2>());
							bp->add_group_input(g, "i_color", TypeInfo::get<vec4>());
							bp->add_group_input(g, "i_normal", TypeInfo::get<vec3>());
							bp->add_group_input(g, "i_coordw", TypeInfo::get<vec3>());
							bp->add_group_output(g, "o_color", TypeInfo::get<vec4>());
							bp->add_group_output(g, "o_normal", TypeInfo::get<vec3>());
							bp->add_group_output(g, "o_metallic", TypeInfo::get<float>());
							bp->add_group_output(g, "o_roughness", TypeInfo::get<float>());
							bp->add_group_output(g, "o_emissive", TypeInfo::get<vec3>());
							bp->add_group_output(g, "o_ao", TypeInfo::get<float>());
							auto n_input = g->find_node("Input"_h);
							auto n_output = g->find_node("Output"_h);
							{
								auto n_defalut = bp->add_node(g, nullptr, "Vec4"_h);
								n_defalut->position = vec2(300, -200);
								bp->add_link(n_defalut->find_output("V"_h), n_output->find_input("o_color"_h));
							}
							bp->add_link(n_input->find_output("i_normal"_h), n_output->find_input("o_normal"_h));
							{
								auto n_defalut = bp->add_node(g, nullptr, "Scalar"_h);
								n_defalut->position = vec2(300, 0);
								bp->add_link(n_defalut->find_output("V"_h), n_output->find_input("o_metallic"_h));
							}
							{
								auto n_defalut = bp->add_node(g, nullptr, "Scalar"_h);
								n_defalut->position = vec2(300, 80);
								bp->add_link(n_defalut->find_output("V"_h), n_output->find_input("o_roughness"_h));
							}
							{
								auto n_defalut = bp->add_node(g, nullptr, "Vec3"_h);
								n_defalut->position = vec2(300, 160);
								bp->add_link(n_defalut->find_output("V"_h), n_output->find_input("o_emissive"_h));
							}
							{
								auto n_defalut = bp->add_node(g, nullptr, "Scalar"_h);
								n_defalut->position = vec2(300, 320);
								*(float*)n_defalut->find_input("V"_h)->data = 1.f;
								bp->add_link(n_defalut->find_output("V"_h), n_output->find_input("o_ao"_h));
							}
							bp->save();
						}
						else
							ImGui::OpenMessageDialog("Failed to create Blueprint", "Blueprint already existed");
					}
				});

			}
			if (ImGui::BeginMenu("New Prefab"))
			{
				if (ImGui::MenuItem("Empty"))
				{
					ImGui::OpenInputDialog("New Prefab", "File Name", [path](bool ok, const std::string& str) {
						if (ok && !str.empty())
						{
							auto fn = path / str;
							fn.replace_extension(L".prefab");
							if (!std::filesystem::exists(fn))
								app.new_prefab(fn);
							else
								ImGui::OpenMessageDialog("Failed to create Prefab", "Prefab already existed");
						}
					});
				}
				if (ImGui::MenuItem("General 3D Scene"))
				{
					ImGui::OpenInputDialog("New Prefab", "File Name", [path](bool ok, const std::string& str) {
						if (ok && !str.empty())
						{
							auto fn = path / str;
							fn.replace_extension(L".prefab");
							if (!std::filesystem::exists(fn))
								app.new_prefab(fn, "general_3d_scene"_h);
							else
								ImGui::OpenMessageDialog("Failed to create Prefab", "Prefab already existed");
						}
					});
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("New Preset"))
			{
				ImGui::OpenInputDialog("New Preset", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".preset");
						if (!std::filesystem::exists(fn))
						{
							std::vector<UdtInfo*> preset_udts;
							for (auto& ui : tidb.udts)
							{
								if (ui.second.name.ends_with("Preset"))
									preset_udts.push_back(&ui.second);
							}
							std::sort(preset_udts.begin(), preset_udts.end(), [](const auto& a, const auto& b) {
								return a->name < b->name;
							});

							std::vector<std::string> names(preset_udts.size());
							for (auto i = 0; i < names.size(); i++)
								names[i] = preset_udts[i]->name;
							ImGui::OpenSelectDialog("New Preset", "Type", names, [preset_udts, fn](int index) {
								if (index != -1)
								{
									auto ui = preset_udts[index];
									auto obj = ui->create_object();
									save_preset_file(fn, obj, ui);
									ui->destroy_object(obj);
								}
							});
						}
						else
							ImGui::OpenMessageDialog("Failed to create Preset", "Preset already existed");
					}
				});
			}
			if (ImGui::MenuItem("New Timeline"))
			{
				ImGui::OpenInputDialog("New Timeline", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".timeline");
						if (!std::filesystem::exists(fn))
						{
							auto timeline = Timeline::create();
							timeline->save(fn);
							delete timeline;
						}
						else
							ImGui::OpenMessageDialog("Failed to create Timeline", "Timeline already existed");
					}
				});
			}
			if (ImGui::MenuItem("New Atlas"))
			{
				auto atlas_path = path / L"atlas_config.ini";
				if (!std::filesystem::exists(atlas_path))
				{
					std::ofstream file(atlas_path);
					file << "size=1024,1024" << std::endl;
					file.close();
				}
			}
			if (ImGui::MenuItem("Import Scene"))
			{
				struct ImportSceneDialog : ImGui::Dialog
				{
					vec3 rotation = vec3(0.f);
					float scaling = 1.f;
					bool only_animations = false;

					std::filesystem::path source_path; // a file or directory
					std::filesystem::path destination;

					static void open(const std::filesystem::path& _destination)
					{
						auto dialog = new ImportSceneDialog;
						dialog->title = "Import Scene";
						dialog->destination = _destination;
						Dialog::open(dialog);
					}

					void draw() override
					{
						bool open = true;
						if (ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_NoSavedSettings))
						{
							auto s = source_path.string();
							if (ImGui::InputText("Source Path (file or directory)", &s))
								source_path = s;
							ImGui::SameLine();
							if (ImGui::Button("..."))
							{
								ImGui::OpenFileDialog("Path", [this](bool ok, const std::filesystem::path& path) {
									if (ok)
										source_path = path;
								});
							}

							ImGui::Text("Destination: %s", destination.string().c_str());
							ImGui::Separator();

							ImGui::InputFloat3("Rotation", &rotation[0]);
							ImGui::InputFloat("Scaling", &scaling);
							ImGui::Checkbox("Only Animations", &only_animations);

							if (ImGui::Button("OK"))
							{
								std::vector<std::filesystem::path> files;
								if (std::filesystem::is_directory(source_path))
								{
									for (auto& e : std::filesystem::recursive_directory_iterator(source_path))
										files.push_back(e.path());
								}
								else if (std::filesystem::is_regular_file(source_path))
									files.push_back(source_path);
								for (auto& p : files)
								{
									auto ext = p.extension();
									ext = SUW::get_lowered(ext.wstring());
									if (is_model_file(ext))
										graphics::import_scene(p, destination, rotation, scaling, only_animations);
								}

								close();
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel"))
								close();

							ImGui::End();
						}
						if (!open)
							close();
					}
				};

				ImportSceneDialog::open(path);
			}
		}
		if (in_cpp)
		{
			if (ImGui::MenuItem("New Header File"))
			{
				ImGui::OpenInputDialog("New Header File", "File Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto fn = path / str;
						fn.replace_extension(L".h");
						if (!std::filesystem::exists(fn))
						{
							std::ofstream file(fn);
							file << "#pragma once" << std::endl;
							file.close();
						}
					}
				});
			}
			if (ImGui::MenuItem("New Class"))
			{
				ImGui::OpenInputDialog("New Class", "Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto h_fn = path / str;
						h_fn.replace_extension(L".h");
						auto cpp_fn = path / str;
						cpp_fn.replace_extension(L".cpp");
						if (!std::filesystem::exists(h_fn) && !std::filesystem::exists(cpp_fn))
						{
							std::ofstream h_file(h_fn);
							h_file << "#pragma once" << std::endl;
							h_file.close();

							std::ofstream cpp_file(cpp_fn);
							cpp_file << std::format("#include \"{}\".h", str) << std::endl;
							cpp_file.close();
						}
					}
				});
			}
			if (ImGui::MenuItem("New Component"))
			{
				ImGui::OpenInputDialog("New Component", "Name", [path](bool ok, const std::string& str) {
					if (ok && !str.empty())
					{
						auto h_fn = path / str;
						h_fn.replace_extension(L".h");
						auto cpp_fn = path / str;
						cpp_fn.replace_extension(L".cpp");
						if (!std::filesystem::exists(h_fn) && !std::filesystem::exists(cpp_fn))
						{
							std::ofstream h_file(h_fn);
							h_file << "#pragma once" << std::endl;
							h_file << std::endl;
							h_file << "#include <flame/universe/component.h>" << std::endl;
							h_file << std::endl;
							h_file << "using namespace flame;" << std::endl;
							h_file << std::endl;
							auto name = str;
							name[0] = std::toupper(name[0]);
							name = 'c' + name;
							h_file << std::format("struct {} : Component", name) << std::endl;
							h_file << "{" << std::endl;
							h_file << "\tstruct Create" << std::endl;
							h_file << "\t{" << std::endl;
							h_file << std::format("\t\tvirtual {}* operator()(EntityPtr) = 0;", name) << std::endl;
							h_file << "\t};" << std::endl;
							h_file << "\t// Reflect static" << std::endl;
							h_file << "\tEXPORT static Create& create;" << std::endl;
							h_file << "};" << std::endl;
							h_file.close();

							std::ofstream cpp_file(cpp_fn);
							cpp_file << std::format("#include \"{}.h\"", str) << std::endl;
							cpp_file << std::endl;
							cpp_file << std::format("struct {0}Create : {0}::Create", name) << std::endl;
							cpp_file << "{" << std::endl;
							cpp_file << std::format("\t{}* operator()(EntityPtr e) override", name) << std::endl;
							cpp_file << "\t{" << std::endl;
							cpp_file << "\t\tif (e == INVALID_POINTER)" << std::endl;
							cpp_file << "\t\t\treturn nullptr;" << std::endl;
							cpp_file << std::format("\t\treturn new {};", name) << std::endl;
							cpp_file << "\t}" << std::endl;
							cpp_file << std::format("}}{}_create;", name) << std::endl;
							cpp_file << std::format("{0}::Create& {0}::create = {0}_create", name) << std::endl;
							cpp_file.close();
						}
					}
					else
						ImGui::OpenMessageDialog("Failed to create Component", "Already have this component");
				});
			}
		}
		if (ImGui::MenuItem("Refresh"))
		{
			for (auto& it : std::filesystem::directory_iterator(path))
			{
				update_thumbnail(it.path());
			}
		}
	};
	explorer.folder_drop_callback = [this](const std::filesystem::path& path) {
		if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
		{
			auto e_src = *(EntityPtr*)payload->Data;
			if (e_src->prefab_instance)
				open_message_dialog("Error", "Entity is already a prefab instance");
			else
			{
				auto fn = std::filesystem::path(e_src->name);
				if (fn.empty())
					fn = L"entity";
				fn += L".prefab";
				fn = path / fn;
				while (true)
				{
					if (!std::filesystem::exists(fn))
						break;
					replace_fn(fn, L"{}_");
				}
				new PrefabInstance(e_src, fn);
				e_src->save(fn);
			}
		}
	};
	using ExplorerItem = graphics::Explorer::Item;
	explorer.item_created_callback = [](ExplorerItem* item) {
		if (auto thumbnails_dir = item->path.parent_path() / L".thumbnails"; std::filesystem::is_directory(thumbnails_dir))
		{
			if (auto thumbnail_path = thumbnails_dir / (item->path.filename().wstring() + L".png"); std::filesystem::exists(thumbnail_path))
			{
				item->icon = graphics::Image::get(thumbnail_path);
				item->icon_releaser = [](graphics::Image* img) {
					graphics::Image::release(img);
				};
			}
		}
		auto ext = item->path.extension();
		if (ext == L".prefab")
		{
			if (!item->icon)
				item->icon = icon_prefab;
		}
		else if (ext == L".fmat")
		{
			if (!item->icon)
				item->icon = icon_material;
		}
	};
	explorer.special_folder_provider = [](const std::filesystem::path& path, std::vector<ExplorerItem*>& out_items) {
		if (path == L"Favorites")
		{
			for (auto& p : app.project_settings.favorites)
			{
				auto i = new ExplorerItem(p);
				out_items.push_back(i);
			}
			return true;
		}
		else if (path == L"Recents")
		{
			for (auto& p : recent_paths)
			{
				auto i = new ExplorerItem(p);
				out_items.push_back(i);
			}
			return true;
		}
		return false;
	};
	explorer.rename_callback = [](const std::filesystem::path& path, const std::string& name) {
		if (!name.empty())
		{
			auto new_path = path.parent_path() / name;
			if (!std::filesystem::exists(new_path))
				std::filesystem::rename(path, new_path);
		}
	};

	reset();
}

void ProjectView::on_draw()
{
	project_window.process_changed_paths();

	if (selection.type == Selection::tPath)
	{
		if (selection_changed)
			explorer.ping(selection.as_path());
	}
	else
		explorer.selected_paths.clear();

	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);
	imgui_window = ImGui::GetCurrentWindow();

	title_context_menu();

	explorer.draw();

	ImGui::End();
	if (!opened)
		delete this;

	selection_changed = false;
}

void ProjectView::reset()
{
	std::vector<std::filesystem::path> paths;
	paths.push_back(std::filesystem::path(L"Favorites=Favorites (F4)"));
	paths.push_back(std::filesystem::path(L"Recents"));
	paths.push_back(std::filesystem::path(project_window.flame_path.native() + L"=flame"));
	if (!project_window.assets_path.empty())
		paths.push_back(project_window.assets_path);
	if (!project_window.code_path.empty())
		paths.push_back(project_window.code_path);

	explorer.reset_n(paths);
}

ProjectWindow::ProjectWindow() :
	Window("Project")
{
	flame_path = Path::get(L"flame");
}

void ProjectWindow::init()
{
	icon_prefab = graphics::Image::get(L"flame\\icon_prefab.png");
	icon_material = graphics::Image::get(L"flame\\icon_material.png");
	icon_mesh = graphics::Image::get(L"flame\\icon_mesh.png");
	icon_armature = graphics::Image::get(L"flame\\icon_armature.png");

	selection.callbacks.add([](uint caller) {
		if (caller != "project"_h)
			selection_changed = true;
	}, "project"_h);

}

View* ProjectWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new ProjectView;
	return nullptr;
}

View* ProjectWindow::open_view(const std::string& name)
{
	return new ProjectView(name);
}

ProjectView* ProjectWindow::first_view() const
{
	return views.empty() ? nullptr : (ProjectView*)views.front().get();
}

void ProjectWindow::reset()
{
	assets_path = L"";
	code_path = L"";

	if (!app.project_path.empty())
	{
		assets_path = app.project_path / L"assets";
		code_path = app.project_path / L"code";
	}

	for (auto& v : views)
	{
		auto pv = (ProjectView*)v.get();
		pv->reset();
	}

	if (flame_file_watcher)
	{
		set_native_event(flame_file_watcher);
		flame_file_watcher = nullptr;
	}
	if (assets_file_watcher)
	{
		set_native_event(assets_file_watcher);
		assets_file_watcher = nullptr;
	}
	if (code_file_watcher)
	{
		set_native_event(code_file_watcher);
		code_file_watcher = nullptr;
	}
	auto file_watcher = [this](FileChangeFlags flags, const std::filesystem::path& path) {
		mtx_changed_paths.lock();
		auto it = changed_paths.find(path);
		if (it == changed_paths.end())
			changed_paths[path] = flags;
		else
			it->second = (FileChangeFlags)(it->second | flags);
		mtx_changed_paths.unlock();
	};
	flame_file_watcher = add_file_watcher(flame_path, file_watcher, true, false);
	if (!app.project_path.empty())
	{
		assets_file_watcher = add_file_watcher(assets_path, file_watcher, true, false);
		code_file_watcher = add_file_watcher(code_path, file_watcher, true, false);
	}
}

void ProjectWindow::ping(const std::filesystem::path& path)
{
	for (auto& v : views)
	{
		auto pv = (ProjectView*)v.get();
		if (pv->explorer.find_item(path))
		{
			if (pv->imgui_window)
				ImGui::FocusWindow((ImGuiWindow*)pv->imgui_window);
			pv->explorer.ping(path);
			return;
		}
	}

	// not found, force the first view to navigate to that path
	if (auto pv = project_window.first_view(); pv)
	{
		pv->explorer.peeding_open_path = path.parent_path();
		if (pv->imgui_window)
			ImGui::FocusWindow((ImGuiWindow*)pv->imgui_window);
		add_event([pv, path]() {
			pv->explorer.ping(path);
			return false; 
		});
	}
}

void ProjectWindow::process_changed_paths()
{
	mtx_changed_paths.lock();
	if (!changed_paths.empty())
	{
		std::vector<std::filesystem::path>								changed_directories;
		std::vector<std::pair<std::filesystem::path, FileChangeFlags>>	changed_files;
		for (auto& p : changed_paths)
		{
			if (!std::filesystem::exists(p.first))
				continue;
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

		for (auto& v : views)
		{
			auto pv = (ProjectView*)v.get();
			auto current_path = pv->explorer.opened_folder ? pv->explorer.opened_folder->path : L"";
			for (auto& p : changed_directories)
			{
				if (auto node = pv->explorer.find_folder(p); node && node->read)
				{
					node->read = false;
					node->read_children();
				}
			}
			if (!current_path.empty())
				pv->explorer.peeding_open_path = current_path;
		}

		std::vector<std::pair<AssetManagemant::Asset*, std::filesystem::path>>	changed_assets;
		std::pair<std::vector<graphics::MaterialPtr>, uint>						materials;
		std::pair<std::vector<graphics::ShaderPtr>, uint>						shaders;
		std::pair<std::vector<graphics::GraphicsPipelinePtr>, uint>				graphics_pipelines;
		std::map<graphics::ShaderPtr, uint>										changed_shaders;
		std::map<graphics::GraphicsPipelinePtr, uint>							changed_pipelines;
		bool																	project_changed = false;
		auto get_materials = [&]() {
			if (materials.second < frames)
			{
				materials.first = graphics::Debug::get_materials();
				materials.second = frames;
			}
		};
		auto get_shaders = [&]() {
			if (shaders.second < frames)
			{
				shaders.first = graphics::Debug::get_shaders();
				shaders.second = frames;
			}
		};
		auto get_graphics_pipelines = [&]() {
			if (graphics_pipelines.second < frames)
			{
				graphics_pipelines.first = graphics::Debug::get_graphics_pipelines();
				graphics_pipelines.second = frames;
			}
		};
		for (auto& p : changed_files)
		{
			if ((p.second & FileModified) || (p.second & FileRemoved) || (p.second & FileRenamed))
			{
				if (auto asset = AssetManagemant::find(p.first); asset)
				{
					if (p.second & FileModified)
					{
						if (auto lwt = std::filesystem::last_write_time(p.first); lwt > asset->lwt)
						{
							changed_assets.emplace_back(asset, p.first);
							asset->lwt = lwt;
						}
					}
					else
						changed_assets.emplace_back(asset, p.first);
				}
				auto ext = p.first.extension();
				if (ext == L".sheet")
				{
					if (p.first.native().starts_with(app.project_static_path.native()))
					{
						if (p.second & FileAdded)
						{
							auto sht = Sheet::get(p.first);
							app.project_static_sheets.push_back(sht);

						}
						else if (p.second & FileRemoved)
						{
							for (auto it = app.project_static_sheets.begin(); it != app.project_static_sheets.end(); it++)
							{
								if ((*it)->filename == p.first)
								{
									Sheet::release(*it);
									app.project_static_sheets.erase(it);
									break;
								}
							}
						}
						else if (p.second & FileRenamed)
						{
							// TODO: is rename avaliable?
						}
					}
				}
				else if (ext == L".bp")
				{
					if (p.second & FileModified)
					{
						get_materials();
						for (auto mat : materials.first)
						{
							if (Path::get(mat->code_file) == p.first)
								mat->generate_code();
						}
					}
				}
				else if (ext == L".prefab")
				{
					if (p.second & FileModified)
					{
						if (app.e_prefab)
						{
							app.e_prefab->backward_traversal([p](EntityPtr e) {
								if (e->prefab_instance && e->prefab_instance->filename == p.first)
								{
									auto ins = get_root_prefab_instance(e);
									std::vector<std::tuple<GUID, std::string, std::string>> staging_values; // guid, target, value
									if (ins)
									{
										for (auto& m : ins->modifications)
										{
											ModificationParsedData data; voidptr obj;
											auto type = e->parse_modification_target(m, data, obj);
											if (auto t = e->find_with_file_id(data.d.guid); t)
											{
												auto& sv = staging_values.emplace_back();
												std::get<0>(sv) = data.d.guid;
												std::get<1>(sv) = m;
												if (type == ModificationAttributeModify)
													std::get<2>(sv) = data.d.attr->serialize(obj);
											}
										}
									}

									empty_entity(e);
									e->load(p.first);

									if (ins)
									{
										for (auto& sv : staging_values)
										{
											if (auto t = e->find_with_file_id(std::get<0>(sv)); t)
											{
												ModificationParsedData data; voidptr obj;
												auto type = e->parse_modification_target(std::get<1>(sv), data, obj);
												if (type == ModificationAttributeModify)
													data.d.attr->unserialize(obj, std::get<2>(sv));
											}
										}
									}

									if (selection.selecting(e))
										selection.clear("project"_h);
								}
							});
						}
					}
				}
				else if (ext == L".glsl")
				{
					if ((p.second & FileModified) || (p.second & FileRenamed))
					{
						get_shaders();
						get_graphics_pipelines();
						for (auto sd : shaders.first)
						{
							for (auto& d : sd->dependencies)
							{
								if (d == p.first)
								{
									changed_shaders[sd] = 1;
									break;
								}
							}
						}
						for (auto pl : graphics_pipelines.first)
						{
							for (auto& d : pl->dependencies)
							{
								if (d == p.first)
								{
									changed_pipelines[pl] = 1;
									break;
								}
							}
						}
					}
				}
				if (ext == L".vert" || ext == L".frag" || ext == L".tesc" || ext == L".tese" || ext == L".geom" ||
					ext == L".comp" || ext == L".task" || ext == L".mesh")
				{
					get_shaders();
					for (auto sd : shaders.first)
					{
						if (sd->filename == p.first)
							changed_shaders[sd]= 1;
					}
				}
				else if (ext == L".pipeline")
				{
					get_graphics_pipelines();
					for (auto pl : graphics_pipelines.first)
					{
						if (pl->filename == p.first)
							changed_pipelines[pl]= 1;
					}
				}
			}

			if ((p.second & FileAdded) || (p.second & FileRemoved) || (p.second & FileRenamed))
			{
				for (auto it = p.first.begin(); it != p.first.end(); it++)
				{
					if (*it == L"cpp")
					{
						project_changed = true;
						break;
					}
				}
			}
		}
		if (!changed_assets.empty())
		{
			// empty the original references to thoese assets, and then assign again
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
								auto& value = *(std::filesystem::path*)a.get_value(c.get(), true);
								if (!value.empty())
								{
									auto s = value.wstring();
									auto abs_value = Path::get(std::filesystem::path(SUW::split(s, '#').front()));
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
					}
					});
				for (auto& a : affected_attributes)
					std::get<1>(a)->set_value(std::get<0>(a), &std::get<2>(a));
			}
		}
		if (!changed_shaders.empty())
		{
			get_graphics_pipelines();
			for (auto& sd : changed_shaders)
			{
				for (auto pl : graphics_pipelines.first)
				{
					for (auto _sd : pl->shaders)
					{
						if (sd.first == _sd)
						{
							changed_pipelines[pl]= 1;
							break;
						}
					}
				}
			}
		}
		if (!changed_shaders.empty() || !changed_pipelines.empty())
			graphics::Queue::get()->wait_idle();
		for (auto& sd : changed_shaders)
			sd.first->recreate();
		for (auto& pl : changed_pipelines)
			pl.first->recreate();
		if (project_changed)
			app.cmake_project();

		changed_paths.clear();
	}
	mtx_changed_paths.unlock();
}