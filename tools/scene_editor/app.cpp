#include "app.h"
#include "window_scene.h"

#include <flame/serialize.h>
#include <imgui.h>
#include <ImFileDialog.h>

std::list<Window*> windows;

Window::Window(const std::string& name) :
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

	e->get_parent()->remove_child(e);
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
	app.set_main_window(graphics::Window::create(nullptr, NativeWindow::create(L"Scene Editor", uvec2(1280, 720), NativeWindowFrame | NativeWindowResizable)));
	app.s_renderer->set_clear_color(vec4(0.2f, 0.2f, 0.2f, 1.f));

	{
		app.imgui_root = Entity::create();
		auto c = cImgui::create();
		c->on_draw([](Capture& c) {
			ImGui::SetCurrentContext((ImGuiContext*)c._current);

			ImGui::BeginMainMenuBar();
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
					ifd::FileDialog::Instance().Open("OpenDialog", "Open a file", "file (*.prefab){.prefab},.*");
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
			ImGui::EndMainMenuBar();

			if (ifd::FileDialog::Instance().IsDone("OpenDialog"))
			{
				if (ifd::FileDialog::Instance().HasResult())
				{
					auto res = ifd::FileDialog::Instance().GetResult();
					printf("OPEN[%s]\n", res.string().c_str());
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
		app.imgui_root->add_component(c);
		app.root->add_child(app.imgui_root);
	}

	//auto script_ins = script::Instance::get_default();
	//script_ins->excute_file(L"camera.lua");
	//script_ins->excute_file(L"cmd.lua");
	//script_ins->excute_file(L"main.lua");
}

int main(int argc, char** args)
{
	app.init(); 

	std::ifstream window_status_i("window_status.txt");
	if (window_status_i.good())
	{
		while (!window_status_i.eof())
		{
			std::string line;
			std::getline(window_status_i, line);
			if (!line.empty())
			{
				for (auto w : windows)
				{
					if (w->name == line)
					{
						w->open();
						break;
					}
				}
			}
		}
		window_status_i.close();
	}

	run([](Capture& c, float) {
		app.update();
	}, Capture());

	std::ofstream window_status_o("window_status.txt");
	for (auto w : windows)
	{
		if (w->e)
			window_status_o << w->name << "\n";
	}
	window_status_o.close();

	return 0;
}
