#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
	std::filesystem::path engine_path;

	graphics::FontAtlas* font_atlas_edit;

	cText* c_code;
	Entity* e_result;
	Entity* e_wait;

	void compile_and_run()
	{
		auto code = w2s(c_code->text());
		std::ofstream file("out.cpp");
		file << "#include <flame/universe/utils/ui.h>\n";
		file << "using namespace flame;";
		file << "using namespace utils;";
		file << "extern \"C\" __declspec(dllexport) void excute(Entity* e)\n{\n";
		file << "	style_set_to_dark;";
		file << "	push_parent(e);";
		file << code;
		file << "	pop_parent();";
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

		typedef void(*F)(Entity*);
		struct Capture
		{
			void* m;
			F f;
		}capture;
		capture.m = nullptr;
		capture.f = nullptr;

		if (std::filesystem::exists(L"out.dll") && std::filesystem::last_write_time(L"out.dll") > std::filesystem::last_write_time(L"out.cpp"))
		{
			capture.m = load_module((get_curr_path().str() + L"/out.dll").c_str());
			if (capture.m)
				capture.f = (F)get_module_func(capture.m, "excute");
		}

		looper().add_event([](void* c, bool*) {
			auto& capture = *(Capture*)c;
			if (capture.m)
			{
				if (capture.f)
					capture.f(app.e_result);
				free_module(capture.m);
			}
			utils::remove_layer(app.e_wait->parent());
		}, Mail::from_t(&capture));
	}
}app;

int main(int argc, char** args)
{
	app.engine_path = getenv("FLAME_PATH");

	app.create("UI Test", Vec2u(600, 400), WindowFrame | WindowResizable, true, app.engine_path);

	const wchar_t* fonts[] = {
		L"c:/windows/fonts/consola.ttf"
	};
	app.font_atlas_edit = graphics::FontAtlas::create(app.graphics_device, 1, fonts);
	app.canvas->add_font(app.font_atlas_edit);

	utils::push_parent(app.root);
		utils::e_begin_splitter(SplitterHorizontal);
			utils::e_begin_layout(LayoutVertical, 4.f);
			utils::c_aligner(SizeFitParent, SizeFitParent);
			app.c_code = utils::e_edit(100.f)->get_component(cText);
			app.c_code->font_atlas = app.font_atlas_edit;
			app.c_code->font_size_ = 17;
			utils::c_aligner(SizeFitParent, SizeFitParent);
			utils::e_button(L"Run", [](void*) {
				looper().add_event([](void*, bool*) {
					app.e_result->remove_children(0, -1);
					app.e_wait = utils::e_begin_dialog();
					auto c_text = utils::e_text(L"Compiling: 0")->get_component(cText);
					auto c_timer = utils::c_timer();
					c_timer->interval = 1.f;
					struct Capture
					{
						cText* text;
						uint time;
					}capture;
					capture.text = c_text;
					capture.time = 0;
					c_timer->set_callback([](void* c) {
						auto& capture = *(Capture*)c;
						capture.time++;
						capture.text->set_text(wfmt(L"Compiling: %d", capture.time).c_str());
					}, Mail::from_t(&capture));
					utils::e_end_dialog();
					add_work([](void*) {
						app.compile_and_run();
					}, Mail());
				}, Mail());
			}, Mail());
			utils::e_end_layout();

			app.e_result = utils::e_element();
			utils::c_aligner(SizeFitParent, SizeFitParent);
			utils::c_layout(LayoutVertical)->item_padding = 4.f;
		utils::e_end_splitter();
	utils::pop_parent();

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
