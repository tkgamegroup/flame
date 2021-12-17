#include "app.h"
#include "selection.h"
#include "window_scene.h"
#include "window_project.h"

std::list<Window*> windows;

Window::Window(std::string_view name) :
	name(name)
{
	windows.push_back(this);
}

void Window::open()
{
	if (lis)
		return;

	lis = app.main_window->add_imgui_callback([this](void* ctx) {
		ImGui::SetCurrentContext((ImGuiContext*)ctx);
		draw();
	});
}

void Window::close()
{
	if (!lis)
		return;

	add_event([this]() {
		app.main_window->remove_imgui_callback(lis);
		return false;
	});
	lis = nullptr;
}

void Window::draw()
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
	app.create("Scene Editor", uvec2(1280, 720), WindowFrame | WindowResizable | WindowMaximized);

	app.main_window->add_imgui_callback([](void* ctx) {
		ImGui::SetCurrentContext((ImGuiContext*)ctx);

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project"))
				ifd::FileDialog::Instance().Open("OpenProjectDialog", "Open a project", "");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			for (auto w : windows)
			{
				auto selected = (bool)w->lis;
				if (ImGui::MenuItem(w->name.c_str(), nullptr, &selected))
					w->open();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Render"))
		{
			if (ImGui::MenuItem("Always Update", nullptr, app.always_update))
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
			{
				auto res = ifd::FileDialog::Instance().GetResult();
				if (!res.has_stem())
				{
					auto str = res.wstring();
					str.pop_back();
					res = str;
				}
				app.open_project(res);
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
		project_path = path;
		selection.clear();

		window_project.reset();
	}
}

int main(int argc, char** args)
{
	auto ap = parse_args(argc, args);

	app.init(); 

	auto settings_i = parse_ini_file(L"settings.ini");
	for (auto& e : settings_i.get_section_entries("opened_windows"))
	{
		for (auto w : windows)
		{
			if (w->name == e.value)
			{
				w->open();
				break;
			}
		}
	}
	for (auto& e : settings_i.get_section_entries("project_path"))
	{
		if (app.project_path.empty())
			app.open_project(e.value);
		break;
	}

	app.run();

	std::ofstream settings_o("settings.ini");
	settings_o << "[opened_windows]\n";
	for (auto w : windows)
	{
		if (w->lis)
			settings_o << w->name << "\n";
	}
	settings_o << "[project_path]\n";
	settings_o << app.project_path.string() << "\n";
	settings_o.close();

	return 0;
}
