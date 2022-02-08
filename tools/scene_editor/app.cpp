#include "app.h"
#include "selection.h"
#include "view_scene.h"
#include "view_project.h"

#include <flame/foundation/system.h>

std::list<View*> views;

View::View(std::string_view name) :
	name(name)
{
	views.push_back(this);
}

void View::open()
{
	if (lis)
		return;

	lis = app.main_window->imgui_callbacks.add([this]() {
		draw();
	});
}

void View::close()
{
	if (!lis)
		return;

	add_event([this]() {
		app.main_window->imgui_callbacks.remove(lis);
		return false;
	});
	lis = nullptr;
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

void App::init()
{
	app.create(true, "Scene Editor", uvec2(1280, 720), WindowFrame | WindowResizable | WindowMaximized);
	app.always_render = false;

	app.main_window->imgui_callbacks.add([this]() {
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project"))
				ifd::FileDialog::Instance().Open("OpenProjectDialog", "Open a project", "");
			if (ImGui::MenuItem("Open Prefab"))
				ifd::FileDialog::Instance().Open("OpenPrefabDialog", "Open a prefab", "Prefab file (*.prefab){.prefab}");
			if (ImGui::MenuItem("New Prefab"))
				ifd::FileDialog::Instance().Save("NewPrefabDialog", "New prefab", "Prefab file (*.prefab){.prefab}");
			if (ImGui::MenuItem("Save Prefab"))
				ifd::FileDialog::Instance().Save("SavePrefabDialog", "Save prefab", "Prefab file (*.prefab){.prefab}");
			if (ImGui::MenuItem("Close"))
				;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Scene"))
		{
			if (ImGui::MenuItem("Create Entity"))
			{
				if (e_prefab)
				{
					static int id = 0;
					auto e = Entity::create();
					e->name = "Entity " + str(id++);
					e_prefab->add_child(e);
				}
			}
			if (ImGui::MenuItem("Remove Entity"))
			{
				if (selection.type == Selection::tEntity)
				{
					selection.entity->parent->remove_child(selection.entity);
					selection.clear();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			for (auto w : views)
			{
				auto selected = (bool)w->lis;
				if (ImGui::MenuItem(w->name.c_str(), nullptr, &selected))
					w->open();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render"))
		{
			if (ImGui::MenuItem("Always Render", nullptr, app.always_update))
			{
				app.always_update = !app.always_update;
				//app.s_renderer->set_always_update(app.always_update);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (ifd::FileDialog::Instance().IsDone("OpenProjectDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
				app.open_project(ifd::FileDialog::Instance().GetResultFormated());
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("OpenPrefabDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
				open_prefab(ifd::FileDialog::Instance().GetResultFormated());
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("NewPrefabDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				auto path = ifd::FileDialog::Instance().GetResultFormated();
				auto e = Entity::create();
				e->name = "Hello";
				e->save(path);
				open_prefab(path);
			}
			ifd::FileDialog::Instance().Close();
		}
		if (ifd::FileDialog::Instance().IsDone("SavePrefabDialog"))
		{
			if (ifd::FileDialog::Instance().HasResult())
			{
				if (e_prefab)
				{
					auto editor_node = e_prefab->find_child("[Editor]");
					if (editor_node)
						editor_node->parent->remove_child(editor_node, false);
					e_prefab->save(ifd::FileDialog::Instance().GetResultFormated());
					if (editor_node)
						e_prefab->add_child(editor_node, 0);
				}
			}
			ifd::FileDialog::Instance().Close();
		}

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
		ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
		ImGui::End();
	});
}

void App::open_project(const std::filesystem::path& path)
{
	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
	{
		if (!project_path.empty())
			directory_lock(project_path, false);

		selection.clear();
		project_path = path;
		directory_lock(project_path, true);
		if (auto p = path / L"assets"; std::filesystem::exists(p))
			Path::set_root(L"assets", p);

		view_project.reset();
	}
}

void App::open_prefab(const std::filesystem::path& path)
{
	prefab_path = path;

	if (e_prefab)
		e_prefab->parent->remove_child(e_prefab);
	e_prefab = Entity::create();
	e_prefab->load(path);
	auto editor_node = Entity::create();
	editor_node->name = "[Editor]";
	editor_node->add_component(th<cNode>());
	editor_node->add_component(th<cCamera>());
	e_prefab->add_child(editor_node, 0);
	app.world->root->add_child(e_prefab);
}

int main(int argc, char** args)
{
	auto ap = parse_args(argc, args);

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
		if (w->lis)
			preferences_o << w->name << "\n";
	}
	if (!app.project_path.empty())
	{
		preferences_o << "[project_path]\n";
		preferences_o << app.project_path.string() << "\n";
	}
	if (app.e_prefab)
	{
		preferences_o << "[opened_prefab]\n";
		preferences_o << app.prefab_path.string() << "\n";
	}
	preferences_o.close();

	return 0;
}
