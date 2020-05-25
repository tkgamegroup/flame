#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

struct MainWindow : App::Window
{
	UI ui;

	MainWindow();
};

MainWindow* main_window = nullptr;

struct MyApp : App
{
	graphics::FontAtlas* font_atlas_edit;

	cText* c_code;
	Entity* e_result;
	Entity* e_wait;

	void* m;
	typedef void(*F)(Entity*);
	F f;

	MyApp()
	{
		m = nullptr;
		f = nullptr;
	}

	void clean_up()
	{
		e_result->remove_children(0, -1);
		if (m)
			free_library(m);

		m = nullptr;
		f = nullptr;
	}

	void compile_and_run()
	{
		auto code = w2s(c_code->text.str());
		std::ofstream file("out.cpp");
		file << "#include <flame/universe/utils/ui.h>\n";
		file << "using namespace flame;";
		file << "using namespace utils;";
		file << "extern \"C\" __declspec(dllexport) void excute(Entity* e)\n{\n";
		file << "	style_set_to_dark;";
		file << "	parents.push(e);";
		file << code;
		file << "	parents.pop();";
		file << "\n}\n";
		file.close();

		std::wstring vs_path = s2w(VS_LOCATION);
		auto cmd = L"\"" + vs_path + L"/VC/Auxiliary/Build/vcvars64.bat\" & ";
		cmd += L"cl ";
		cmd += L"-I\"" + (engine_path / L"include").wstring() + L"\" ";
		{
			std::ifstream cmake_caches(engine_path / L"build/CMakeCache.txt");
			while (!cmake_caches.eof())
			{
				std::string line;
				std::getline(cmake_caches, line);
				static std::regex reg_pugixml_include_dir(R"(PUGIXML_INCLUDE_DIR:PATH\=(.*))");
				static std::regex reg_njson_include_dir(R"(NJSON_INCLUDE_DIR:PATH\=(.*))");
				std::smatch res;
				if (std::regex_search(line, res, reg_pugixml_include_dir))
					cmd += L"-I\"" + s2w(res[1].str()) + L"\" ";
				else if (std::regex_search(line, res, reg_njson_include_dir))
					cmd += L"-I\"" + s2w(res[1].str()) + L"\" ";
			}
			cmake_caches.close();
		}
		cmd += L"out.cpp -W0 -EHsc -std:c++latest -LD -O2 ";
		cmd += L"\"" + (engine_path / L"bin/debug/flame_foundation.lib").wstring() + L"\" ";
		cmd += L"\"" + (engine_path / L"bin/debug/flame_graphics.lib").wstring() + L"\" ";
		cmd += L"\"" + (engine_path / L"bin/debug/flame_sound.lib").wstring() + L"\" ";
		cmd += L"\"" + (engine_path / L"bin/debug/flame_universe.lib").wstring() + L"\" ";
		exec_and_redirect_to_std_output(nullptr, (wchar_t*)cmd.c_str());

		if (std::filesystem::exists(L"out.dll") && std::filesystem::last_write_time(L"out.dll") > std::filesystem::last_write_time(L"out.cpp"))
		{
			m = load_library((get_curr_path().str() + L"/out.dll").c_str());
			if (m)
				f = (F)get_library_func(m, "excute");
		}

		looper().add_event([](Capture&) {
			if (app.f)
				app.f(app.e_result);
			remove_layer(app.e_wait->parent);
		}, Capture().set_thiz(this));
	}
}app;

MainWindow::MainWindow() :
	App::Window(&app, true, true, "Code Live", Vec2u(800, 400), WindowFrame | WindowResizable)
{
	main_window = this;

	canvas->add_font(app.font_atlas_edit);

	setup_as_main_window();

	ui.init(world);

	ui.parents.push(root);
		ui.e_begin_splitter(SplitterHorizontal);
			ui.e_begin_layout(LayoutVertical, 4.f);
			ui.c_aligner(AlignMinMax, AlignMinMax);
			auto c_edit = ui.e_edit(100.f)->get_component(cEdit);
			c_edit->select_all_on_dbclicked = false;
			c_edit->select_all_on_focus = false;
			app.c_code = c_edit->text;
			app.c_code->font_atlas = app.font_atlas_edit;
			app.c_code->font_size = 17;
			ui.c_aligner(AlignMinMax, AlignMinMax);
			ui.e_button(L"Run", [](Capture&) {
				looper().add_event([](Capture&) {
					auto& ui = main_window->ui;
					app.clean_up();
					app.e_wait = ui.e_begin_dialog();
					auto c_text = ui.e_text(L"Compiling: 0")->get_component(cText);
					auto c_timer = ui.c_timer();
					c_timer->interval = 1.f;
					struct Capturing
					{
						cText* text;
						uint time;
					}capture;
					capture.text = c_text;
					capture.time = 0;
					c_timer->set_callback([](Capture& c) {
						auto& capture = c.data<Capturing>();
						capture.time++;
						capture.text->set_text(wfmt(L"Compiling: %d", capture.time).c_str());
					}, Capture().set_data(&capture));
					ui.e_end_dialog();
					add_work([](Capture&) {
						app.compile_and_run();
					}, Capture());
				}, Capture());
			}, Capture());
			ui.e_end_layout();

			app.e_result = ui.e_element();
			ui.c_aligner(AlignMinMax, AlignMinMax);
			ui.c_layout(LayoutVertical)->item_padding = 4.f;
		ui.e_end_splitter();
	ui.parents.pop();
}

int main(int argc, char** args)
{
	app.create();

	const wchar_t* fonts[] = {
		L"c:/windows/fonts/consola.ttf"
	};
	app.font_atlas_edit = graphics::FontAtlas::create(app.graphics_device, 1, fonts);

	new MainWindow;

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
