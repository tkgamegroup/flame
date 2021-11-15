#include "app.h"
#include "selection.h"
#include "window_scene.h"
#include "window_project.h"

#include <flame/serialize.h>

std::list<Window*> windows;

Window::Window(std::string_view name) :
	name(name)
{
	windows.push_back(this);
}

void Window::open()
{
	if (e)
		return;

	e = Entity::create();
	auto c = cImgui::create();
	c->on_draw([](Capture& c) {
		ImGui::SetCurrentContext((ImGuiContext*)c._current);
		c.thiz<Window>()->draw();
	}, Capture().set_thiz(this));
	e->add_component(c);
	app.imgui_root->add_child(e);
}

void Window::close()
{
	if (!e)
		return;

	add_event([](Capture& c) {
		auto e = c.thiz<Entity>();
		e->get_parent()->remove_child(e);
	}, Capture().set_thiz(e));
	e = nullptr;
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

MyApp app;

void MyApp::init()
{
	app.create();
	app.set_main_window(graphics::Window::create(nullptr, NativeWindow::create("Scene Editor", uvec2(1280, 720), WindowFrame | WindowResizable | WindowMaximized)), true);
	app.s_renderer->set_clear_color(vec4(0.2f, 0.4f, 0.7f, 1.f));
	app.s_imgui->set_clear_color(vec4(0.2f, 0.2f, 0.2f, 1.f));

	app.imgui_root->get_component_t<cImgui>()->on_draw([](Capture& c) {
		ImGui::SetCurrentContext((ImGuiContext*)c._current);

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
				auto selected = (bool)w->e;
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
				app.s_renderer->set_always_update(app.always_update);
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
	}, Capture());
}

void MyApp::open_project(const std::filesystem::path& path)
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

	run([]() {
		app.update();
		return true;
	});

	std::ofstream settings_o("settings.ini");
	settings_o << "[opened_windows]\n";
	for (auto w : windows)
	{
		if (w->e)
			settings_o << w->name << "\n";
	}
	settings_o << "[project_path]\n";
	settings_o << app.project_path.string() << "\n";
	settings_o.close();

	return 0;
}
