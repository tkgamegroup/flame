#include "app.h"
#include "selection.h"
#include "view_scene.h"
#include "view_project.h"

#include <flame/xml.h>
#include <flame/foundation/system.h>
#include <flame/foundation/typeinfo.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>
#include <flame/universe/systems/renderer.h>

std::list<View*> views;

View::View(std::string_view name) :
	name(name)
{
	views.push_back(this);
}

void View::open()
{
	if (opened)
		return;
	opened = true;

	graphics::gui_callbacks.add([this]() {
		draw();
	}, (uint)this);
}

void View::close()
{
	if (!opened)
		return;
	opened = false;

	add_event([this]() {
		graphics::gui_callbacks.remove((uint)this);
		return false;
	});
}

void View::draw()
{
	bool open = true;
	ImGui::Begin(name.c_str(), &open);
	on_draw();
	ImGui::End();

	if (!open)
		close();
}

App app;

struct NavMeshTest
{
	bool open = false;

	vec3 start = vec3(0.f);
	vec3 end = vec3(0.f);
	std::vector<vec3> points;
}navmesh_test;

void App::init()
{
	create(true, "Scene Editor", uvec2(1280, 720), WindowFrame | WindowResizable | WindowMaximized);
	world->update_components = false;
	always_render = false;
	renderer->type = sRenderer::CameraLight;

	auto root = world->root.get();
	root->add_component(th<cNode>());
	e_editor = Entity::create();
	e_editor->name = "[Editor]";
	e_editor->add_component(th<cNode>());
	e_editor->add_component(th<cCamera>());
	root->add_child(e_editor);

	for (auto& v : views)
		v->init();

	graphics::gui_callbacks.add([this]() {
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
			if (ImGui::MenuItem("New Prefab"))
			{
				ImGui::OpenFileDialog("New Prefab", [this](bool ok, const std::filesystem::path& path) {
					if (ok)
					{
						auto e = Entity::create();
						e->save(path);
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
			if (ImGui::MenuItem("Save Prefab"))
			{
				if (e_prefab)
					e_prefab->save(prefab_path);
			}
			if (ImGui::MenuItem("Close"))
				;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Create Empty"))
				cmd_create_entity();
			if (ImGui::MenuItem("Create Node"))
				cmd_create_entity(nullptr, "node"_h);
			if (ImGui::MenuItem("Create Cube"))
				cmd_create_entity(nullptr, "cube"_h);
			if (ImGui::MenuItem("Create Sphere"))
				cmd_create_entity(nullptr, "sphere"_h);
			if (ImGui::MenuItem("Create Light"))
				cmd_create_entity(nullptr, "light"_h);
			if (ImGui::MenuItem("Create Camera"))
				cmd_create_entity(nullptr, "camera"_h);
			if (ImGui::MenuItem("Delete"))
				cmd_delete_entity();
			if (ImGui::MenuItem("Focus To Selected (F)"))
				view_scene.focus_to_selected();
			if (ImGui::MenuItem("Selected To Focus (G)"))
				view_scene.selected_to_focus();
			ImGui::Separator();
			if (ImGui::BeginMenu("NavMesh"))
			{
				if (ImGui::MenuItem("Generate"))
					sScene::instance()->generate_nav_mesh();
				if (ImGui::MenuItem("Test", nullptr, &navmesh_test.open))
				{
					auto node = e_editor->get_component_i<cNode>(0);
					if (!navmesh_test.open)
						node->drawers.remove("navmesh_test"_h);
					else
					{
						node->drawers.add([&](sRendererPtr renderer, uint pass) {
							if (pass == "lines"_h)
							{
								{
									std::vector<vec3> points;
									points.push_back(navmesh_test.start - vec3(1, 0, 0));
									points.push_back(navmesh_test.start + vec3(1, 0, 0));
									points.push_back(navmesh_test.start - vec3(0, 0, 1));
									points.push_back(navmesh_test.start + vec3(0, 0, 1));
									renderer->draw_line(points.data(), points.size(), cvec4(0, 255, 0, 255));
								}
								{
									std::vector<vec3> points;
									points.push_back(navmesh_test.end - vec3(1, 0, 0));
									points.push_back(navmesh_test.end + vec3(1, 0, 0));
									points.push_back(navmesh_test.end - vec3(0, 0, 1));
									points.push_back(navmesh_test.end + vec3(0, 0, 1));
									renderer->draw_line(points.data(), points.size(), cvec4(0, 0, 255, 255));
								}
								if (!navmesh_test.points.empty())
									renderer->draw_line(navmesh_test.points.data(), navmesh_test.points.size(), cvec4(255, 0, 0, 255));
							}
						}, "navmesh_test"_h);
					}
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Show"))
		{
			ImGui::MenuItem("AABB", nullptr, &view_scene.show_AABB);
			ImGui::MenuItem("Axis", nullptr, &view_scene.show_axis);
			ImGui::MenuItem("Bones", nullptr, &view_scene.show_bones);
			ImGui::MenuItem("Navigation", nullptr, &view_scene.show_navigation);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			for (auto w : views)
			{
				auto selected = (bool)w->opened;
				if (ImGui::MenuItem(w->name.c_str(), nullptr, &selected))
					w->open();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render"))
		{
			if (ImGui::MenuItem("Shaded", nullptr, renderer->type == sRenderer::Shaded))
				renderer->type = sRenderer::Shaded;
			if (ImGui::MenuItem("Camera Light", nullptr, renderer->type == sRenderer::CameraLight))
				renderer->type = sRenderer::CameraLight;
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

		auto add_tool_button = [](Tool tool, uint icon, const std::string& id = "", float rotate = 0.f) {
			ImGui::SameLine();
			auto name = graphics::FontAtlas::icon_s(icon);
			if (!id.empty())
				name += "##" + id;
			if (tool == app.tool)
			{
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 0, 1));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2);
			}
			if (rotate != 0.f)
				ImGui::BeginRotation(rotate);
			auto clicked = ImGui::Button(name.c_str());
			if (tool == app.tool)
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
		if (add_tool_button(ToolSelect, "arrow-pointer"_h))
			tool = ToolSelect;
		if (add_tool_button(ToolMove, "arrows-up-down-left-right"_h))
			tool = ToolMove;
		if (add_tool_button(ToolRotate, "rotate"_h))
			tool = ToolRotate;
		if (add_tool_button(ToolScale, "down-left-and-up-right-to-center"_h))
			tool = ToolScale;
		ImGui::SameLine();
		const char* tool_mode_names[] = {
			"Local",
			"World"
		};
		ImGui::SetNextItemWidth(100.f);
		ImGui::Combo("Mode", (int*)&tool_mode, tool_mode_names, countof(tool_mode_names));
		ImGui::SameLine();
		ImGui::Dummy(vec2(0.f, 20.f));

		static cTerrainPtr terrain_tool_target = nullptr;
		if (add_tool_button(ToolTerrainUp, "mound"_h, "up"))
			tool = ToolTerrainUp;
		if (add_tool_button(ToolTerrainDown, "mound"_h, "down", 180.f))
			tool = ToolTerrainDown;
		if (add_tool_button(ToolTerrainPaint, "paintbrush"_h))
			tool = ToolTerrainPaint;

		ImGui::SameLine();
		ImGui::Dummy(vec2(50.f, 20.f));
		if (!e_playing)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
			if (add_tool_button(ToolNone, "play"_h))
				cmd_play();
			ImGui::PopStyleColor();
		}
		else
		{
			if (!paused)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
				if (add_tool_button(ToolNone, "pause"_h))
					cmd_pause();
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
				if (add_tool_button(ToolNone, "play"_h))
					cmd_play();
				ImGui::PopStyleColor();
			}
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			if (add_tool_button(ToolNone, "stop"_h))
				cmd_stop();
			ImGui::PopStyleColor();
		}
		// toolbar end

		ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		ImGui::End();

		if (ImGui::IsKeyPressed(Keyboard_F5))
		{
			if (!e_playing)
				cmd_play();
			else
				cmd_stop();
		}

		if (navmesh_test.open)
		{
			ImGui::Begin("NavMesh Test", &navmesh_test.open);
			static int v = 0;
			ImGui::TextUnformatted("use ctrl+click to set start/end");
			ImGui::RadioButton("Start", &v, 0);
			ImGui::TextUnformatted(("    " + str(navmesh_test.start)).c_str());
			ImGui::RadioButton("End", &v, 1);
			ImGui::TextUnformatted(("    " + str(navmesh_test.end)).c_str());
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsKeyDown(Keyboard_Ctrl))
			{
				if (v == 0)
					navmesh_test.start = view_scene.hovering_pos;
				else
					navmesh_test.end = view_scene.hovering_pos;
				if (distance(navmesh_test.start, navmesh_test.end) > 0.f)
					navmesh_test.points = sScene::instance()->calc_nav_path(navmesh_test.start, navmesh_test.end);
			}
			ImGui::End();
			if (!navmesh_test.open)
				e_editor->get_component_i<cNode>(0)->drawers.remove("navmesh_test"_h);
		}
	});
}

void App::new_project(const std::filesystem::path& path)
{
	if (!std::filesystem::is_empty(path))
	{
		wprintf(L"cannot create project: %s is not an empty directory\n", path.c_str());
		return;
	}

	auto project_name = path.filename().string();

	auto assets_path = path / L"assets";
	std::filesystem::create_directories(assets_path);

	pugi::xml_document main_prefab;
	{
		auto n_prefab = main_prefab.append_child("prefab");
		n_prefab.append_attribute("file_id").set_value(generate_guid().c_str());
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
				n_item.append_attribute("file_id").set_value(generate_guid().c_str());
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

/// Reflect ctor
struct cMain : Component
{{
	void start() override;

	struct Create
	{{
		virtual cMainPtr operator()(EntityPtr) = 0;
	}};
	/// Reflect static
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
	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
	{
		auto assets_path = path / L"assets";
		if (std::filesystem::exists(assets_path))
		{
			if (!project_path.empty())
				directory_lock(project_path, false);

			Path::set_root(L"assets", assets_path);

			selection.clear();
			project_path = path;
			directory_lock(project_path, true);

			auto cpp_path = path / L"bin/debug/cpp.dll";
			if (std::filesystem::exists(cpp_path))
				tidb.load(cpp_path);

			view_project.reset(assets_path);
		}
	}
}

void App::open_prefab(const std::filesystem::path& path)
{
	prefab_path = path;

	if (e_prefab)
		e_prefab->parent->remove_child(e_prefab);
	e_prefab = Entity::create();
	e_prefab->load(path);
	world->root->add_child(e_prefab);
}

bool App::cmd_create_entity(EntityPtr dst, uint type)
{
	if (!dst)
		dst = e_prefab;
	if (!dst)
		return false;
	static int id = 0;
	auto e = Entity::create();
	e->name = "Entity " + str(id++);
	switch (type)
	{
	case "empty"_h: break;
	case "node"_h:
		e->add_component(th<cNode>());
		break;
	case "cube"_h:
		e->add_component(th<cNode>());
		((cMeshPtr)e->add_component(th<cMesh>()))->set_mesh_name("standard_cube");
		break;
	case "sphere"_h:
		e->add_component(th<cNode>());
		((cMeshPtr)e->add_component(th<cMesh>()))->set_mesh_name("standard_sphere");
		break;
	case "light"_h:
		e->add_component(th<cNode>());
		e->add_component(th<cLight>());
		break;
	case "camera"_h:
		e->add_component(th<cNode>());
		e->add_component(th<cCamera>());
		break;
	}
	dst->add_child(e);
	return true;
}

bool App::cmd_delete_entity(EntityPtr e)
{
	if (!e && selection.type == Selection::tEntity)
		e = selection.entity();
	if (!e)
		return false;
	if (e == e_prefab)
		return false;
	if (!e->prefab && get_prefab_instance(e))
	{
		app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
		return false;
	}
	add_event([e]() {
		e->parent->remove_child(e);
		return false;
	});
	selection.clear();
	return true;
}

bool App::cmd_play()
{
	if (!e_playing && e_prefab)
	{
		add_event([this]() {
			e_playing = e_prefab->copy();
			e_prefab->parent->remove_child(e_prefab, false);
			world->root->add_child(e_playing);
			world->update_components = true;
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
	return true;
}

bool App::cmd_stop()
{
	if (!e_playing)
		return false;
	add_event([this]() {
		e_playing->parent->remove_child(e_playing);
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

PrefabInstance* get_prefab_instance(EntityPtr e)
{
	PrefabInstance* ret = nullptr;
	while (e)
	{
		if (e->prefab)
			ret = e->prefab.get();
		e = e->parent;
	}
	return ret;
}

int main(int argc, char** args)
{
	srand(time(0));

	auto ap = parse_args(argc, args);
	if (ap.has("-fixed_render_target_size"))
		view_scene.fixed_render_target_size = true;

	app.init();

	std::filesystem::path preferences_path = L"preferences.ini";

	auto preferences_i = parse_ini_file(preferences_path);
	for (auto& e : preferences_i.get_section_entries("opened_windows"))
	{
		for (auto w : views)
		{
			if (w->name == e.value)
			{
				w->open();
				break;
			}
		}
	}
	for (auto& e : preferences_i.get_section_entries("project_path"))
	{
		app.open_project(e.value);
		break;
	}
	for (auto& e : preferences_i.get_section_entries("opened_folder"))
	{
		view_project.explorer.peeding_open_path = e.value;
		break;
	}
	for (auto& e : preferences_i.get_section_entries("opened_prefab"))
	{
		app.open_prefab(e.value);
		break;
	}

	app.run();

	std::ofstream preferences_o(preferences_path);
	preferences_o << "[opened_windows]\n";
	for (auto w : views)
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
