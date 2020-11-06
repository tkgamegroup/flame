#include <flame/graphics/model.h>
#include <flame/universe/app.h>
#include <flame/universe/components/command.h>
#include <flame/universe/components/custom_drawing.h>
#include <flame/universe/components/camera.h>

using namespace flame;
using namespace graphics;

App g_app;

auto test_prefab = std::filesystem::path(L"tests/scene_test");

int main(int argc, char** args)
{
	g_app.create();

	auto w = new GraphicsWindow(&g_app, "Universe Test", Vec2u(600, 400), WindowFrame | WindowResizable, true, true);

	w->canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	{
		w->canvas->set_model_resource(-1, Model::create(LR"(D:\character\man.fmod)"), "man");
		Entity::register_prefab(LR"(D:\character\man.prefab)", "main_character");
	}
	{
		auto t = Image::create(graphics::Device::get_default(), L"D:/terrain/height.png", false, graphics::ImageUsageTransferSrc);
		w->canvas->set_texture_resource(-1, t->get_view(0), nullptr, "terrain_height_map");
	}
	{
		auto t = Image::create(graphics::Device::get_default(), L"D:/terrain/normal.png", false, graphics::ImageUsageTransferSrc);
		w->canvas->set_texture_resource(-1, t->get_view(0), nullptr, "terrain_normal_map");
	}
	{
		auto t = Image::create(graphics::Device::get_default(), L"D:/terrain/color.png", true);
		w->canvas->set_texture_resource(-1, t->get_view(0), nullptr, "terrain_color_map");
	}

	{
		auto c = cCommand::create();
		c->add_processor([](Capture& c, const char* _cmd) {
			auto cmd = std::string(_cmd);
			if (cmd == "view_terrain_normal")
				g_app.main_window->render_preferences->set_terrain_render(graphics::RenderNormal);
			else if (cmd == "view_terrain_wireframe")
				g_app.main_window->render_preferences->set_terrain_render(graphics::RenderWireframe);
			else if (cmd == "view_physics_on")
				g_app.main_window->s_physic_world->set_visualization(true);
			else if (cmd == "view_physics_off")
				g_app.main_window->s_physic_world->set_visualization(false);
		}, Capture());
		w->root->add_component(c);
		auto e = Entity::create();
		e->load(test_prefab.c_str());
		{
			auto n = e->find_child("camera");
			if (n)
				w->s_renderer->set_camera(n->get_component_t<cCamera>());
		}
		//e->save(L"d:/1.prefab");
		w->root->add_child(e);
	}

	//w->s_physic_world->set_visualization(true);

	//add_file_watcher(res_path.c_str(), [](Capture& c, FileChangeType, const wchar_t* filename) {
	//	auto path = std::filesystem::path(filename);
	//	if (path.filename() == test_prefab)
	//	{
	//		auto e = Entity::create();
	//		e->load(filename);
	//		auto root = c.thiz<Entity>();
	//		root->remove_all_children();
	//		root->add_child(e);
	//	}
	//}, Capture().set_thiz(root), false, false);

	looper().add_event([](Capture& c) {
		printf("%d\n", looper().get_fps());
		c._current = nullptr;
	}, Capture(), 1.f);

	g_app.run();

	return 0;
}
