#include "app.h"
#include "selection.h"
#include "history.h"
#include "view_scene.h"
#include "view_project.h"
#include "view_inspector.h"

#include <flame/xml.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/foundation/typeinfo_serialize.h>
#include <flame/universe/draw_data.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/components/dir_light.h>
#include <flame/universe/components/nav_scene.h>
#include <flame/universe/systems/renderer.h>

App app;

vec3 App::get_snap_pos(const vec3& _pos)
{
	auto pos = _pos;
	if (move_snap)
	{
		pos /= move_snap_value;
		pos -= fract(pos);
		pos *= move_snap_value;
	}
	return pos;
}

static Entity* editor_selecting_entity = nullptr;

struct Preferences
{
	bool use_flame_debugger = false; // use flame visual studio project debugger or use opened project one

};
static Preferences preferences;

static std::vector<std::function<bool()>> dialogs;

void App::init()
{
	create("Scene Editor", uvec2(1280, 720), WindowFrame | WindowResizable | WindowMaximized, true, graphics_debug, graphics_configs);
	graphics::gui_set_clear(true, vec4(0.f));
	world->update_components = false;
	input->transfer_events = false;
	always_render = false;
	renderer->mode = sRenderer::CameraLight;

	auto root = world->root.get();
	root->add_component_t<cNode>();
	e_editor = Entity::create();
	e_editor->name = "[Editor]";
	e_editor->add_component_t<cNode>();
	e_editor->add_component_t<cCamera>();
	root->add_child(e_editor);

	for (auto& v : graphics::gui_views)
		v->init();

	graphics::gui_callbacks.add([this]() {
		editor_selecting_entity = selection.type == Selection::tEntity ? selection.as_entity() : nullptr;

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Project"))
			{
				ImGui::OpenFileDialog("New Project", [this](bool ok, const std::filesystem::path& path) {
					if (ok)
						new_project(path);
				});
			}
			if (ImGui::MenuItem("Open Project"))
			{
				ImGui::OpenFileDialog("Open Project", [this](bool ok, const std::filesystem::path& path) {
					if (ok)
						open_project(path);
				});
			}
			if (ImGui::MenuItem("Close Project"))
				close_project();
			ImGui::Separator();
			if (ImGui::MenuItem("New Prefab"))
			{
				ImGui::OpenFileDialog("New Prefab", [this](bool ok, const std::filesystem::path& path) {
					if (ok)
					{
						new_prefab(path);
						open_prefab(path);
					}
				});
			}
			if (ImGui::MenuItem("Open Prefab"))
			{
				ImGui::OpenFileDialog("Open Prefab", [this](bool ok, const std::filesystem::path& path) {
					if (ok)
						open_prefab(path);
				});
			}
			if (ImGui::MenuItem("Save Prefab (Ctrl+S)"))
				save_prefab();
			if (ImGui::MenuItem("Close Prefab"))
				close_prefab();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo (Ctrl+Z)"))
				cmd_undo();
			if (ImGui::MenuItem("Redo (Ctrl+Y)"))
				cmd_redo();
			if (ImGui::MenuItem(std::format("Clear Histories ({})", (int)histories.size()).c_str()))
			{
				history_idx = -1;
				histories.clear();
			}
			ImGui::Separator();
			show_entities_menu();
			ImGui::Separator();
			if (ImGui::MenuItem("Clear Selections"))
				selection.clear("app"_h);
			if (ImGui::MenuItem("Select Parent"))
				;
			if (ImGui::MenuItem("Select Children"))
				;
			if (ImGui::MenuItem("Invert Siblings"))
				;
			if (ImGui::MenuItem("Focus To Selected (F)"))
				view_scene.focus_to_selected();
			if (ImGui::MenuItem("Selected To Focus (G)"))
				view_scene.selected_to_focus();
			ImGui::Separator();
			if (ImGui::BeginMenu("NavMesh"))
			{
				struct GenerateDialog
				{
					bool open = false;

					float agent_radius = 0.6f;
					float agent_height = 1.8f;
					float walkable_climb = 0.5f;
					float walkable_slope_angle = 45.f;
				};
				static GenerateDialog generate_dialog;
				if (ImGui::MenuItem("Generate"))
				{
					if (e_prefab)
					{
						dialogs.push_back([&]() {
							if (!generate_dialog.open)
							{
								generate_dialog.open = true;
								ImGui::OpenPopup("NavMesh Generate");
							}

							if (ImGui::BeginPopupModal("NavMesh Generate"))
							{
								ImGui::InputFloat("Agent Radius", &generate_dialog.agent_radius);
								ImGui::InputFloat("Agent Height", &generate_dialog.agent_height);
								ImGui::InputFloat("Walkable Climb", &generate_dialog.walkable_climb);
								ImGui::InputFloat("Walkable Slope Angle", &generate_dialog.walkable_slope_angle);
								if (ImGui::Button("Generate"))
								{
									sScene::instance()->generate_navmesh(generate_dialog.agent_radius, generate_dialog.agent_height, generate_dialog.walkable_climb, generate_dialog.walkable_slope_angle);
									ImGui::CloseCurrentPopup();
									generate_dialog.open = false;
								}
								ImGui::SameLine();
								if (ImGui::Button("Cancel"))
								{
									ImGui::CloseCurrentPopup();
									generate_dialog.open = false;
								}
								ImGui::End();
							}
							return generate_dialog.open;
							});
					}
				}

				if (ImGui::MenuItem("Generate Using cNavScene's values"))
				{
					if (e_prefab)
					{
						if (auto comp = e_prefab->find_component_recursively(th<cNavScene>()); comp)
						{
							auto nav_scene = (cNavScenePtr)comp;
							sScene::instance()->generate_navmesh(nav_scene->agent_radius, nav_scene->agent_height, nav_scene->walkable_climb, nav_scene->walkable_slope_angle);
						}
					}
				}

				struct TestDialog
				{
					bool open = false;

					vec3 start = vec3(0.f);
					vec3 end = vec3(0.f);
					std::vector<vec3> points;
				};
				static TestDialog test_dialog;
				if (ImGui::MenuItem("Test", nullptr, &test_dialog.open))
				{
					auto node = e_editor->get_component_i<cNode>(0);
					if (test_dialog.open)
					{
						dialogs.push_back([&]() {
							if (test_dialog.open)
							{
								ImGui::Begin("NavMesh Test", &test_dialog.open);
								static int v = 0;
								ImGui::TextUnformatted("use ctrl+click to set start/end");
								ImGui::RadioButton("Start", &v, 0);
								ImGui::TextUnformatted(("    " + str(test_dialog.start)).c_str());
								ImGui::RadioButton("End", &v, 1);
								ImGui::TextUnformatted(("    " + str(test_dialog.end)).c_str());
								if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsKeyDown(Keyboard_Ctrl))
								{
									if (v == 0)
										test_dialog.start = view_scene.hovering_pos;
									else
										test_dialog.end = view_scene.hovering_pos;
									if (distance(test_dialog.start, test_dialog.end) > 0.f)
										test_dialog.points = sScene::instance()->query_navmesh_path(test_dialog.start, test_dialog.end);
								}
								ImGui::End();

								{
									std::vector<vec3> points;
									points.push_back(test_dialog.start - vec3(1, 0, 0));
									points.push_back(test_dialog.start + vec3(1, 0, 0));
									points.push_back(test_dialog.start - vec3(0, 0, 1));
									points.push_back(test_dialog.start + vec3(0, 0, 1));
									sRenderer::instance()->draw_primitives("LineList"_h, points.data(), points.size(), cvec4(0, 255, 0, 255));
								}
								{
									std::vector<vec3> points;
									points.push_back(test_dialog.end - vec3(1, 0, 0));
									points.push_back(test_dialog.end + vec3(1, 0, 0));
									points.push_back(test_dialog.end - vec3(0, 0, 1));
									points.push_back(test_dialog.end + vec3(0, 0, 1));
									sRenderer::instance()->draw_primitives("LineList"_h, points.data(), points.size(), cvec4(0, 0, 255, 255));
								}
								if (!test_dialog.points.empty())
									sRenderer::instance()->draw_primitives("LineList"_h, test_dialog.points.data(), test_dialog.points.size(), cvec4(255, 0, 0, 255));
							}
							return test_dialog.open;
							});
					}
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Preferences"))
			{
				struct PreferencesDialog
				{
					bool open = false;
				};
				static PreferencesDialog preferences_dialog;
				dialogs.push_back([&]() {
					if (!preferences_dialog.open)
					{
						preferences_dialog.open = true;
						ImGui::OpenPopup("Preferences");
					}

					if (ImGui::BeginPopupModal("Preferences"))
					{
						ImGui::Checkbox("Use Flame Debugger", &preferences.use_flame_debugger);
						if (ImGui::Button("Close"))
						{
							preferences_dialog.open = false;
							ImGui::CloseCurrentPopup();
						}
						ImGui::End();
					}
					return preferences_dialog.open;
				});
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Open In VS"))
			{
				auto vs_path = get_special_path("Visual Studio Installation Location");
				auto devenv_path = vs_path / L"Common7\\IDE\\devenv.exe";
				auto sln_path = project_path / L"build";
				sln_path = glob_files(sln_path, L".sln")[0];
				exec(devenv_path, std::format(L"\"{}\"", sln_path.wstring()));
			}
			if (ImGui::MenuItem("Attach Debugger"))
				vs_automate({ L"attach_debugger" });
			if (ImGui::MenuItem("Detach Debugger"))
				vs_automate({ L"detach_debugger" });
			if (ImGui::MenuItem("Do CMake"))
				cmake_project();
			if (ImGui::MenuItem("Build (Ctrl+B)"))
				build_project();
			if (ImGui::MenuItem("Clean"))
			{
				if (!project_path.empty())
				{
					auto cpp_path = project_path / L"bin/debug/cpp.dll";
					cpp_path.replace_extension(L".pdb");
					if (std::filesystem::exists(cpp_path))
						std::filesystem::remove(cpp_path);
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			for (auto w : graphics::gui_views)
			{
				auto selected = (bool)w->opened;
				if (ImGui::MenuItem(w->name.c_str(), nullptr, &selected))
					w->open();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render"))
		{
			if (ImGui::MenuItem("Shaded", nullptr, renderer->mode == sRenderer::Shaded))
				renderer->mode = sRenderer::Shaded;
			if (ImGui::MenuItem("Camera Light", nullptr, renderer->mode == sRenderer::CameraLight))
				renderer->mode = sRenderer::CameraLight;
			if (ImGui::MenuItem("Albedo Data", nullptr, renderer->mode == sRenderer::AlbedoData))
				renderer->mode = sRenderer::AlbedoData;
			if (ImGui::MenuItem("Normal Data", nullptr, renderer->mode == sRenderer::NormalData))
				renderer->mode = sRenderer::NormalData;
			if (ImGui::MenuItem("Metallic Data", nullptr, renderer->mode == sRenderer::MetallicData))
				renderer->mode = sRenderer::MetallicData;
			if (ImGui::MenuItem("Roughness Data", nullptr, renderer->mode == sRenderer::RoughnessData))
				renderer->mode = sRenderer::RoughnessData;
			if (ImGui::MenuItem("IBL Value", nullptr, renderer->mode == sRenderer::IBLValue))
				renderer->mode = sRenderer::IBLValue;
			if (ImGui::MenuItem("Fog Value", nullptr, renderer->mode == sRenderer::FogValue))
				renderer->mode = sRenderer::FogValue;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug"))
		{
			struct UIStatusDialog
			{
				bool open = false;

			};
			static UIStatusDialog ui_status_dialog;

			if (ImGui::MenuItem("UI Status", nullptr, &ui_status_dialog.open))
			{
				if (ui_status_dialog.open)
				{
					dialogs.push_back([&]() {
						if (ui_status_dialog.open)
						{
							ImGui::Begin("UI Status", &ui_status_dialog.open);
							ImGui::Text("Want Capture Mouse: %d", (int)graphics::gui_want_mouse());
							ImGui::Text("Want Capture Keyboard: %d", (int)graphics::gui_want_keyboard());
							ImGui::End();
						}
						return ui_status_dialog.open;
					});
				}
			}
			if (ImGui::MenuItem("Send Debug Cmd"))
			{
				ImGui::OpenInputDialog("Send Debug Cmd", "Cmd", [](bool ok, const std::string& str) {
					if (ok)
						sRenderer::instance()->send_debug_string(str);
				}, "", true);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
		ImGui::PopStyleVar(2);

		auto tool_button = [](const std::string& name, bool selected = false, float rotate = 0.f) {
			ImGui::SameLine();
			if (selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 0, 1));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2);
			}
			if (rotate != 0.f)
				ImGui::BeginRotation(rotate);
			auto clicked = ImGui::Button(name.c_str());
			if (selected)
			{
				ImGui::PopStyleColor();
				ImGui::PopStyleVar();
			}
			if (rotate != 0.f)
				ImGui::EndRotation();
			return clicked;
		};

		// toolbar begin
		ImGui::Dummy(vec2(0.f, 20.f));
		if (tool_button(graphics::FontAtlas::icon_s("arrow-pointer"_h), app.tool == ToolSelect))
			tool = ToolSelect;
		if (tool_button(graphics::FontAtlas::icon_s("arrows-up-down-left-right"_h), app.tool == ToolMove))
			tool = ToolMove;
		if (tool_button(graphics::FontAtlas::icon_s("rotate"_h), app.tool == ToolRotate))
			tool = ToolRotate;
		if (tool_button(graphics::FontAtlas::icon_s("down-left-and-up-right-to-center"_h), app.tool == ToolScale))
			tool = ToolScale;
		ImGui::SameLine();
		const char* tool_pivot_names[] = {
			"Individual",
			"Center"
		};
		const char* tool_mode_names[] = {
			"Local",
			"World"
		};
		ImGui::SetNextItemWidth(100.f);
		ImGui::Combo("##pivot", (int*)&tool_pivot, tool_pivot_names, countof(tool_pivot_names));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.f);
		ImGui::Combo("##mode", (int*)&tool_mode, tool_mode_names, countof(tool_mode_names));
		bool* p_snap = nullptr;
		float* p_snap_value = nullptr;
		switch (tool)
		{
		case ToolMove:
			p_snap = &move_snap;
			p_snap_value = view_scene.element_targets.empty() ? &move_snap_value : &move_snap_2d_value;
			break;
		case ToolRotate:
			p_snap = &rotate_snap;
			p_snap_value = &rotate_snap_value;
			break;
		case ToolScale:
			p_snap = &scale_snap;
			p_snap_value = &scale_snap_value;
			break;
		}
		ImGui::SameLine();
		if (p_snap)
		{
			ImGui::Checkbox("Snap", p_snap);
			if (*p_snap)
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(80.f);
				ImGui::InputFloat("##snap_value", p_snap_value);
			}
		}
		ImGui::SameLine();
		if (tool_button(graphics::FontAtlas::icon_s("floppy-disk"_h)))
			save_prefab();
		ImGui::SameLine();
		ImGui::Dummy(vec2(0.f, 20.f));

		if (e_editing)
		{
			if (auto terrain = e_editing->get_component_t<cTerrain>(); terrain)
			{
				if (tool_button(graphics::FontAtlas::icon_s("mound"_h) + "##up", app.tool == ToolTerrainUp))
					tool = ToolTerrainUp;
				if (tool_button(graphics::FontAtlas::icon_s("mound"_h) + "##down", app.tool == ToolTerrainDown, 180.f))
					tool = ToolTerrainDown;
				if (tool_button(graphics::FontAtlas::icon_s("paintbrush"_h), app.tool == ToolTerrainPaint))
					tool = ToolTerrainPaint;
			}
			if (auto tile_map = e_editing->get_component_t<cTileMap>(); tile_map)
			{
				if (tool_button(graphics::FontAtlas::icon_s("up-long"_h), app.tool == ToolTileMapLevelUp))
					tool = ToolTileMapLevelUp;
				if (tool_button(graphics::FontAtlas::icon_s("down-long"_h), app.tool == ToolTileMapLevelDown))
					tool = ToolTileMapLevelDown;
				if (tool_button(graphics::FontAtlas::icon_s("stairs"_h), app.tool == ToolTileMapSlope))
					tool = ToolTileMapSlope;
			}
		}

		ImGui::SameLine();
		ImGui::Dummy(vec2(50.f, 20.f));
		ImGui::SameLine();
		if (e_editing)
		{
			if (tool_button(graphics::FontAtlas::icon_s("right-from-bracket"_h), false, 180.f))
			{
				e_editing = nullptr;
				selection.lock = false;
			}
			else
			{
				ImGui::SameLine();
				ImGui::Text("[%s]", e_editing->name.c_str());
			}
		}
		else
		{
			if (!e_playing && !e_preview)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
				if (tool_button(graphics::FontAtlas::icon_s("play"_h) + " Build And Play"))
				{
					build_project();
					add_event([this]() {
						cmd_play();
						return false;
						}, 0.f, 3);
				}
				if (tool_button(graphics::FontAtlas::icon_s("play"_h)))
					cmd_play();
				if (tool_button(graphics::FontAtlas::icon_s("circle-play"_h)))
					cmd_start_preview(selection.type == Selection::tEntity ? selection.as_entity() : e_prefab);
				ImGui::PopStyleColor();
			}
			else
			{
				if (e_playing)
				{
					if (!paused)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
						if (tool_button(graphics::FontAtlas::icon_s("pause"_h)))
							cmd_pause();
						ImGui::PopStyleColor();
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
						if (tool_button(graphics::FontAtlas::icon_s("play"_h)))
							cmd_play();
						ImGui::PopStyleColor();
					}
				}
				else if (e_preview)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 1, 1));
					if (tool_button(graphics::FontAtlas::icon_s("rotate"_h)))
						cmd_restart_preview();
					ImGui::PopStyleColor();
				}
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
				if (tool_button(graphics::FontAtlas::icon_s("stop"_h)))
				{
					if (e_playing)
						cmd_stop();
					else if (e_preview)
						cmd_stop_preview();
				}
				ImGui::PopStyleColor();
				if (e_preview)
				{
					ImGui::SameLine();
					ImGui::Text("[%s]", e_preview->name.c_str());
				}
			}
		}

		// toolbar end

		ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();

		auto& io = ImGui::GetIO();
		if (ImGui::IsKeyPressed(Keyboard_Tab))
		{
			if (e_editing)
			{
				e_editing = nullptr;
				selection.lock = false;
			}
			else
			{
				if (selection.type == Selection::tEntity)
				{
					e_editing = selection.as_entity();
					selection.lock = true;
				}
			}
		}
		if (ImGui::IsKeyPressed(Keyboard_F5))
		{
			if (!e_playing)
				cmd_play();
			else
			{
				if (e_playing)
					cmd_stop();
				else if (e_preview)
					cmd_stop_preview();
			}
		}
		if (ImGui::IsKeyPressed(Keyboard_F6))
		{
			if (!e_preview)
				cmd_start_preview(selection.type == Selection::tEntity ? selection.as_entity() : e_prefab);
			else
				cmd_restart_preview();
		}
		if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_S))
			save_prefab();
		if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_B))
			build_project();
		if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_Z))
			cmd_undo();
		if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_Y))
			cmd_redo();

		if (e_preview)
		{
			e_preview->forward_traversal([](EntityPtr e) {
				if (!e->global_enable)
					return;
				for (auto& c : e->components)
				{
					if (c->enable)
						c->update();
				}
			});
			render_frames++;
		}

		for (auto it = dialogs.begin(); it != dialogs.end();)
		{
			if (!(*it)())
				it = dialogs.erase(it);
			else
				it++;
		}
	});
}

void App::new_project(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path))
	{
		wprintf(L"cannot create project: %s not exists\n", path.c_str());
		return;
	}
	if (!std::filesystem::is_empty(path))
	{
		wprintf(L"cannot create project: %s is not an empty directory\n", path.c_str());
		return;
	}

	auto project_name = path.filename().string();

	auto assets_path = path / L"assets";
	std::filesystem::create_directories(assets_path);

	auto temp_path = path / L"temp";
	std::filesystem::create_directories(temp_path);

	pugi::xml_document main_prefab;
	{
		auto n_prefab = main_prefab.append_child("prefab");
		n_prefab.append_attribute("file_id").set_value(generate_guid().to_string().c_str());
		n_prefab.append_attribute("name").set_value("Main");
		{
			auto n_components = n_prefab.append_child("components");
			n_components.append_child("item").append_attribute("type_hash").set_value("flame::cNode"_h);
			n_components.append_child("item").append_attribute("type_hash").set_value("cMain"_h);
		}
		{
			auto n_children = n_prefab.append_child("children");
			{
				auto n_item = n_children.append_child("item");
				n_item.append_attribute("file_id").set_value(generate_guid().to_string().c_str());
				n_item.append_attribute("name").set_value("Camera");
				{
					auto n_components = n_item.append_child("components");
					n_components.append_child("item").append_attribute("type_hash").set_value("flame::cNode"_h);
					n_components.append_child("item").append_attribute("type_hash").set_value("flame::cCamera"_h);
				}
			}
		}
	}
	main_prefab.save_file((assets_path / L"main.prefab").c_str());

	auto cpp_path = path / L"cpp";
	std::filesystem::create_directories(cpp_path);

	std::ofstream main_h(cpp_path / L"main.h");
	const auto main_h_content =
		R"^^^(
#pragma once

#include <flame/universe/component.h>

using namespace flame;

FLAME_TYPE(cMain)

// Reflect ctor
struct cMain : Component
{{
	void start() override;

	struct Create
	{{
		virtual cMainPtr operator()(EntityPtr) = 0;
	}};
	// Reflect static
	EXPORT static Create& create;
}};

)^^^";
	main_h << std::format(main_h_content, project_name);
	main_h.close();

	std::ofstream main_cpp(cpp_path / L"main.cpp");
	const auto main_cpp_content =
		R"^^^(
#include <flame/universe/entity.h>

#include "main.h"

void cMain::start()
{{
	printf("Hello World\n");
}}

struct cMainCreate : cMain::Create
{{
	cMainPtr operator()(EntityPtr e) override
	{{
		if (e == INVALID_POINTER)
			return nullptr;
		return new cMain;
	}}
}}cMain_create;
cMain::Create& cMain::create = cMain_create;

EXPORT void* cpp_info()
{{
	auto uinfo = universe_info();
	cMain::create((EntityPtr)INVALID_POINTER);
	return nullptr;
}}

)^^^";
	main_cpp << std::format(main_cpp_content, project_name);
	main_cpp.close();

	std::ofstream app_cpp(path / L"app.cpp");
	const auto app_cpp_content =
		R"^^^(
#include <flame/universe/application.h>

using namespace flame;

UniverseApplication app;

IMPORT void* cpp_info();

int main()
{{
	auto info = cpp_info();
	Path::set_root(L"assets", std::filesystem::current_path() / L"assets");
	app.create(false, "{0}", uvec2(1280, 720), WindowFrame | WindowResizable);
	app.world->root->load(L"assets/main.prefab");
	app.node_renderer->bind_window_targets();
	app.run();
	return 0;
}}

)^^^";
	app_cpp << std::format(app_cpp_content, project_name);
	app_cpp.close();

	auto cmake_path = path / L"CMakeLists.txt";
	std::ofstream cmake_lists(cmake_path);
	const auto cmake_content =
		R"^^^(
cmake_minimum_required(VERSION 3.16.4)
set(flame_path "$ENV{{FLAME_PATH}}")
include("${{flame_path}}/utils.cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(-W0 -std:c++latest)

project({0})

set_output_dir("${{CMAKE_SOURCE_DIR}}/bin")

set(GLM_INCLUDE_DIR "")
set(IMGUI_DIR "")
file(STRINGS "${{flame_path}}/build/CMakeCache.txt" flame_cmake_cache)
foreach(s ${{flame_cmake_cache}})
	if(GLM_INCLUDE_DIR STREQUAL "")
		string(REGEX MATCH "GLM_INCLUDE_DIR:PATH=(.*)" res "${{s}}")
		if(NOT res STREQUAL "")
			set(GLM_INCLUDE_DIR ${{CMAKE_MATCH_1}})
		endif()
	endif()
	if(IMGUI_DIR STREQUAL "")
		string(REGEX MATCH "IMGUI_DIR:PATH=(.*)" res "${{s}}")
		if(NOT res STREQUAL "")
			set(IMGUI_DIR ${{CMAKE_MATCH_1}})
		endif()
	endif()
endforeach()

file(GLOB_RECURSE source_files "cpp/*.h*" "cpp/*.c*")
add_library(cpp SHARED ${{source_files}})
target_compile_definitions(cpp PUBLIC USE_IMGUI)
target_compile_definitions(cpp PUBLIC "IMPORT=__declspec(dllimport)")
target_compile_definitions(cpp PUBLIC "EXPORT=__declspec(dllexport)")
target_compile_definitions(cpp PUBLIC IMGUI_USER_CONFIG="${config_file}")
target_include_directories(cpp PUBLIC "${{GLM_INCLUDE_DIR}}")
target_include_directories(cpp PUBLIC "${{IMGUI_DIR}}")
target_include_directories(cpp PUBLIC "${{flame_path}}/include")
target_link_libraries(cpp "${{flame_path}}/bin/debug/imgui.lib")
target_link_libraries(cpp "${{flame_path}}/bin/debug/flame_foundation.lib")
target_link_libraries(cpp "${{flame_path}}/bin/debug/flame_graphics.lib")
target_link_libraries(cpp "${{flame_path}}/bin/debug/flame_universe.lib")

file(GENERATE OUTPUT "$<TARGET_FILE_DIR:cpp>/cpp.typedesc" CONTENT "${{CMAKE_CURRENT_SOURCE_DIR}}/cpp" TARGET cpp)
add_custom_command(TARGET cpp POST_BUILD COMMAND "${{flame_path}}/bin/debug/typeinfogen.exe" $<TARGET_FILE:cpp>)

add_executable({0} "app.cpp")
target_link_libraries({0} cpp)
set_target_properties({0} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${{CMAKE_CURRENT_SOURCE_DIR}}")

file(GLOB dll_files "${{flame_path}}/bin/debug/*.dll")
foreach(f IN LISTS dll_files)
	add_custom_command(TARGET {0} POST_BUILD COMMAND ${{CMAKE_COMMAND}} -E copy_if_different ${{f}} $(TargetDir))
endforeach()
file(GLOB typeinfo_files "${{flame_path}}/bin/debug/*.typeinfo")
foreach(f IN LISTS typeinfo_files)
	add_custom_command(TARGET {0} POST_BUILD COMMAND ${{CMAKE_COMMAND}} -E copy_if_different ${{f}} $(TargetDir))
endforeach()

)^^^";
	cmake_lists << std::format(cmake_content, project_name);
	cmake_lists.close();

	auto build_path = path / L"build";
	std::filesystem::create_directories(build_path);
	exec(L"", std::format(L"cmake -S {} -B {}", path.c_str(), build_path.c_str()));
}

void App::open_project(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
		return;
	if (e_playing)
		return;

	close_project();

	project_path = path;
	directory_lock(project_path, true);

	auto assets_path = project_path / L"assets";
	if (std::filesystem::exists(assets_path))
	{
		Path::set_root(L"assets", assets_path);
		view_project.reset();
	}
	else
		assert(0);

	project_settings.load(project_path / L"project_settings.xml");
	for (auto& p : project_settings.favorites)
		p = Path::get(p);

	switch (project_settings.build_after_open)
	{
	case 0:
		load_project_cpp();
		break;
	case 1:
		build_project();
		break;
	case 2:
	{
		load_project_cpp();

		struct BuildProjectDialog
		{
			bool open = false;
		};
		static BuildProjectDialog build_project_dialog;
		add_event([]() {
			dialogs.push_back([&]() {
				if (!build_project_dialog.open)
				{
					build_project_dialog.open = true;
					ImGui::OpenPopup("Build Project");
				}

				if (ImGui::BeginPopupModal("Build Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Checkbox("Use Flame Debugger", &preferences.use_flame_debugger);
					if (ImGui::Button("OK"))
					{
						app.build_project();

						ImGui::CloseCurrentPopup();
						build_project_dialog.open = false;
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
						build_project_dialog.open = false;
					}
					ImGui::End();
				}

				return build_project_dialog.open;
			});
			return false;
		}, 0.f, 2);
	}
		break;
	}
}

void App::cmake_project()
{
	auto build_path = project_path;
	build_path /= L"build";
	shell_exec(L"cmake", std::format(L"-S \"{}\" -B \"{}\"", project_path.wstring(), build_path.wstring()), true);
}

void App::build_project()
{
	if (project_path.empty() || e_playing)
		return;

	auto old_prefab_path = prefab_path;
	if (!old_prefab_path.empty())
		close_prefab();

	add_event([this, old_prefab_path]() {
		unload_project_cpp();

		focus_window(get_console_window());

		cmake_project();

		vs_automate({ L"detach_debugger" });
		auto cpp_project_path = project_path / L"build\\cpp.vcxproj";
		auto vs_path = get_special_path("Visual Studio Installation Location");
		auto msbuild_path = vs_path / L"Msbuild\\Current\\Bin\\MSBuild.exe";
		auto cwd = std::filesystem::current_path();
		std::filesystem::current_path(cpp_project_path.parent_path());
		printf("\n");
		auto cl = std::format(L"\"{}\" {}", msbuild_path.wstring(), cpp_project_path.filename().wstring());
		_wsystem(cl.c_str());
		std::filesystem::current_path(cwd);
		vs_automate({ L"attach_debugger" });
		 
		load_project_cpp();

		if (!old_prefab_path.empty())
			open_prefab(old_prefab_path);

		return false;
	});
}

void App::close_project()
{
	if (e_playing)
		return;

	close_prefab();

	if (!project_path.empty())
		directory_lock(project_path, false);
	project_path = L"";

	Path::set_root(L"assets", L"");
	view_project.reset();
	unload_project_cpp();
}

void App::new_prefab(const std::filesystem::path& path, uint type)
{
	auto e = Entity::create();
	switch (type)
	{
	case "general_3d_scene"_h:
		e->add_component_t<cNode>();
		e->add_component_t<cNavScene>()->generate_delay_frames = 1;
		auto e_camera = Entity::create();
		e_camera->name = "Camera";
		e_camera->add_component_t<cNode>();
		e_camera->add_component_t<cCamera>();
		e->add_child(e_camera);
		auto e_light = Entity::create();
		e_light->name = "Directional Light";
		e_light->add_component_t<cNode>()->set_eul(vec3(45.f, -60.f, 0.f));
		e_light->add_component_t<cDirLight>();
		e->add_child(e_light);
		auto e_plane = Entity::create();
		e_plane->name = "Plane";
		e_plane->tag = e_plane->tag | TagMarkNavMesh;
		e_plane->add_component_t<cNode>();
		e_plane->add_component_t<cMesh>()->set_mesh_and_material(L"standard_plane", L"default");
		e->add_child(e_plane);
		break;
	}
	e->save(path);
	delete e;
}

void App::open_prefab(const std::filesystem::path& path)
{
	if (e_playing || ev_open_prefab)
		return;
	close_prefab();
	prefab_path = path;
	prefab_unsaved = false;

	ev_open_prefab = add_event([this]() {
		e_prefab = Entity::create();
		e_prefab->load(prefab_path);
		world->root->add_child(e_prefab);
		ev_open_prefab = nullptr;
		return false;
	});
}

bool App::save_prefab()
{
	if (e_prefab && prefab_unsaved)
	{
		e_prefab->save(prefab_path);
		prefab_unsaved = false;
	}
	return true;
}

void App::close_prefab()
{
	if (e_playing)
		return;
	e_preview = nullptr;
	prefab_path = L"";
	selection.clear("app"_h);

	if (e_prefab)
	{
		auto e = e_prefab;
		add_event([this, e]() {
			e->remove_from_parent();
			return false;
		});
		e_prefab = nullptr;
	}
}

void App::load_project_cpp()
{
	auto cpp_path = project_path / L"bin/debug/cpp.dll";
	if (std::filesystem::exists(cpp_path))
		project_cpp_library = tidb.load(cpp_path);
}

void App::unload_project_cpp()
{
	if (project_cpp_library)
	{
		tidb.unload(project_cpp_library);
		project_cpp_library = nullptr;
	}
}

void App::open_file_in_vs(const std::filesystem::path& path)
{
	vs_automate({ L"open_file", path.wstring() });
}

void App::vs_automate(const std::vector<std::wstring>& cl)
{
	std::filesystem::path automation_path = getenv("FLAME_PATH");
	automation_path /= L"bin/debug/vs_automation.exe";
	std::wstring cl_str;
	if (cl[0] == L"attach_debugger" || cl[0] == L"detach_debugger")
	{
		if (!preferences.use_flame_debugger)
			cl_str = L"-p " + project_path.filename().wstring();
		cl_str += L" -c " + cl[0];
		cl_str += L" " + wstr(getpid());
	}
	else if (cl[0] == L"open_file")
	{
		cl_str = L"-p " + project_path.filename().wstring();
		cl_str += L" -c open_file " + cl[1];
	}
	wprintf(L"vs automate: %s\n", cl_str.c_str());
	shell_exec(automation_path, cl_str, true);
}

bool App::cmd_undo()
{
	if (history_idx < 0)
		return false;
	histories[history_idx]->undo();
	history_idx--;
	return true;
}

bool App::cmd_redo()
{
	if (history_idx + 1 >= histories.size())
		return false;
	history_idx++;
	histories[history_idx]->redo();
	return true;
}

bool App::cmd_new_entities(std::vector<EntityPtr>&& es, uint type)
{
	if (es.empty())
	{
		if (e_playing)
			es.push_back(e_playing);
		else if (e_prefab)
			es.push_back(e_prefab);
		else
			return false;
	}
	static int id = 0;
	for (auto t : es)
	{
		auto e = Entity::create();
		e->name = "Entity " + str(id++);
		switch (type)
		{
		case "empty"_h:
			break;
		case "node"_h:
			e->add_component_t<cNode>();
			break;
		case "plane"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cMesh>()->set_mesh_and_material(L"standard_plane", L"default");
			break;
		case "cube"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cMesh>()->set_mesh_and_material(L"standard_cube", L"default");
			break;
		case "sphere"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cMesh>()->set_mesh_and_material(L"standard_sphere", L"default");
			break;
		case "cylinder"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cMesh>()->set_mesh_and_material(L"standard_cylinder", L"default");
			break;
		case "tri_prism"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cMesh>()->set_mesh_and_material(L"standard_tri_prism", L"default");
			break;
		case "dir_light"_h:
			e->add_component_t<cNode>()->set_eul(vec3(45.f, -60.f, 0.f));
			e->add_component_t<cDirLight>();
			break;
		case "pt_light"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cPtLight>();
			break;
		case "camera"_h:
			e->add_component_t<cNode>();
			e->add_component_t<cCamera>();
			break;
		case "element"_h:
			e->add_component_t<cElement>();
			break;
		case "image"_h:
			e->add_component_t<cElement>();
			e->add_component_t<cImage>();
			break;
		case "text"_h:
			e->add_component_t<cElement>();
			e->add_component_t<cText>();
			break;
		}
		t->add_child(e);
	}
	prefab_unsaved = true;
	return true;
}

bool App::cmd_delete_entities(std::vector<EntityPtr>&& es)
{
	if (es.empty())
		return false;
	for (auto t : es)
	{
		if (t == e_prefab || !t->prefab_instance && get_root_prefab_instance(t))
		{
			app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
			return false;
		}
	}
	for (auto t : es)
	{
		add_event([t]() {
			t->remove_from_parent();
			return false;
		});
	}
	selection.clear("app"_h);
	prefab_unsaved = true;
	return true;
}

bool App::cmd_duplicate_entities(std::vector<EntityPtr>&& es)
{
	if (es.empty())
		return false;
	for (auto t : es)
	{
		if (t == e_prefab || !t->prefab_instance && get_root_prefab_instance(t))
		{
			app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
			return false;
		}
	}
	std::vector<EntityPtr> new_entities;
	for (auto t : es)
	{
		auto new_one = t->copy();
		new_entities.push_back(new_one);
		t->parent->add_child(new_one);
	}
	selection.select(new_entities, "app"_h);
	prefab_unsaved = true;
	return true;
}

bool App::cmd_play()
{
	if (e_editing || e_preview)
		return false;
	if (!e_playing && e_prefab)
	{
		add_event([this]() {
			e_prefab->remove_from_parent(false);
			e_playing = e_prefab->copy();
			world->root->add_child(e_playing);
			world->update_components = true;
			input->transfer_events = true;
			always_render = true;
			paused = false;
			auto& camera_list = cCamera::list();
			if (camera_list.size() > 1)
				view_scene.camera_idx = 1;
			return false;
		});
		return true;
	}
	else if (paused)
	{
		paused = false;
		world->update_components = true;
		input->transfer_events = true;
		return true;
	}
	return false;
}

bool App::cmd_pause()
{
	if (!e_playing || paused)
		return false;
	paused = true;
	world->update_components = false;
	input->transfer_events = false;
	return true;
}

bool App::cmd_stop()
{
	if (!e_playing)
		return false;
	add_event([this]() {
		e_playing->remove_from_parent();
		e_playing = nullptr;
		world->root->add_child(e_prefab);
		world->update_components = false;
		always_render = false;
		auto& camera_list = cCamera::list();
		if (camera_list.size() > 0)
		{
			view_scene.camera_idx = 0;
			sRenderer::instance()->camera = camera_list.front();
		}
		else
		{
			view_scene.camera_idx = -1;
			sRenderer::instance()->camera = nullptr;
		}
		return false;
	});

	return true;
}

bool App::cmd_start_preview(EntityPtr e)
{
	if (e_editing)
		return false;
	if (e_preview)
		cmd_stop_preview();

	e_preview = e;

	if (e_preview->enable)
	{
		e_preview->set_enable(false);
		e_preview->set_enable(true);
	}
	e_preview->forward_traversal([](EntityPtr e) {
		if (!e->global_enable)
			return;
		for (auto& c : e->components)
		{
			if (c->enable)
				c->start();
		}
	});

	return true;
}

bool App::cmd_stop_preview()
{
	if (!e_preview)
		return false;

	if (e_preview->enable)
	{
		e_preview->set_enable(false);
		e_preview->set_enable(true);
	}

	e_preview = nullptr;

	return true;
}

bool App::cmd_restart_preview()
{
	if (e_editing)
		return false;
	if (!e_preview)
		return false;

	auto e = e_preview;
	cmd_stop_preview();
	cmd_start_preview(e);

	return true;
}

void App::open_message_dialog(const std::string& title, const std::string& message)
{
	if (title == "[RestructurePrefabInstanceWarnning]")
	{
		ImGui::OpenMessageDialog("Cannot restructure Prefab Instance", 
			"You cannot add/remove/reorder entity or component in Prefab Instance\n"
			"Edit it in that prefab");
	}
	else
		ImGui::OpenMessageDialog(title, message);
}

void App::show_entities_menu()
{
	if (ImGui::MenuItem("New Empty"))
		cmd_new_entities(selection.get_entities());
	if (ImGui::BeginMenu("New 3D"))
	{
		if (ImGui::MenuItem("Node"))
			cmd_new_entities(selection.get_entities(), "node"_h);
		if (ImGui::MenuItem("Plane"))
			cmd_new_entities(selection.get_entities(), "plane"_h);
		if (ImGui::MenuItem("Cube"))
			cmd_new_entities(selection.get_entities(), "cube"_h);
		if (ImGui::MenuItem("Sphere"))
			cmd_new_entities(selection.get_entities(), "sphere"_h);
		if (ImGui::MenuItem("Cylinder"))
			cmd_new_entities(selection.get_entities(), "cylinder"_h);
		if (ImGui::MenuItem("Triangular Prism"))
			cmd_new_entities(selection.get_entities(), "tri_prism"_h);
		if (ImGui::MenuItem("Directional Light"))
			cmd_new_entities(selection.get_entities(), "dir_light"_h);
		if (ImGui::MenuItem("Point Light"))
			cmd_new_entities(selection.get_entities(), "pt_light"_h);
		if (ImGui::MenuItem("Camera"))
			cmd_new_entities(selection.get_entities(), "camera"_h);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("New 2D"))
	{
		if (ImGui::MenuItem("Element"))
			cmd_new_entities(selection.get_entities(), "element"_h);
		if (ImGui::MenuItem("Image"))
			cmd_new_entities(selection.get_entities(), "image"_h);
		if (ImGui::MenuItem("Text"))
			cmd_new_entities(selection.get_entities(), "text"_h);
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Duplicate (Shift+D)"))
		cmd_duplicate_entities(selection.get_entities());
	if (ImGui::MenuItem("Delete (Del)"))
		cmd_delete_entities(selection.get_entities());
}

int main(int argc, char** args)
{
	srand(time(0));

	auto ap = parse_args(argc, args);
	if (ap.has("-fixed_render_target_size"))
		view_scene.fixed_render_target_size = true;
	if (ap.has("-dont_use_mesh_shader"))
		app.graphics_configs.emplace_back("mesh_shader"_h, 0);
	if (ap.has("-replace_renderpass_attachment_dont_care_to_load"))
		app.graphics_configs.emplace_back("replace_renderpass_attachment_dont_care_to_load"_h, 1);

	app.init();

	std::filesystem::path preferences_path = L"preferences.ini";

	auto preferences_i = parse_ini_file(preferences_path);
	for (auto& e : preferences_i.get_section_entries(""))
	{
		if (e.key == "use_flame_debugger")
			preferences.use_flame_debugger = s2t<bool>(e.values[0]);
	}
	for (auto& e : preferences_i.get_section_entries("opened_windows"))
	{
		for (auto w : graphics::gui_views)
		{
			if (w->name == e.values[0])
			{
				w->open();
				break;
			}
		}
	}
	for (auto& e : preferences_i.get_section_entries("project_path"))
	{
		app.open_project(e.values[0]);
		break;
	}
	for (auto& e : preferences_i.get_section_entries("opened_folder"))
	{
		view_project.explorer.peeding_open_path = e.values[0];
		break;
	}
	for (auto& e : preferences_i.get_section_entries("opened_prefab"))
	{
		app.open_prefab(e.values[0]);
		break;
	}

	app.run();

	for (auto& p : app.project_settings.favorites)
		p = Path::reverse(p);
	app.project_settings.save();

	std::ofstream preferences_o(preferences_path);
	preferences_o << "use_flame_debugger=" + str(preferences.use_flame_debugger) << "\n";
	preferences_o << "[opened_windows]\n";
	for (auto w : graphics::gui_views)
	{
		if (w->opened)
			preferences_o << w->name << "\n";
	}
	if (!app.project_path.empty())
	{
		preferences_o << "[project_path]\n";
		preferences_o << app.project_path.string() << "\n";
	}
	if (view_project.explorer.opened_folder)
	{
		preferences_o << "[opened_folder]\n";
		preferences_o << view_project.explorer.opened_folder->path.string() << "\n";
	}
	if (app.e_prefab)
	{
		preferences_o << "[opened_prefab]\n";
		preferences_o << app.prefab_path.string() << "\n";
	}
	preferences_o.close();

	return 0;
}
