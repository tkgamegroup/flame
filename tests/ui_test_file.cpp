#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable, true, engine_path);

	app.world->add_object(app.font_atlas_pixel);

	utils::set_current_entity(app.root);
	utils::c_event_receiver();
	utils::c_layout();
	utils::push_font_atlas(app.font_atlas_pixel);

	utils::push_parent(app.root);

	utils::e_text(L"");
	utils::c_aligner(AlignxLeft, AlignyBottom);
	add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, Mail::from_p(utils::current_entity()->get_component(cText)));

	utils::next_element_pos = Vec2f(16.f, 28.f);
	utils::e_begin_layout(LayoutVertical, 16.f);
		auto e_scene = utils::e_element();
		{
			auto c_element = e_scene->get_component(cElement);
			c_element->size_ = 500.f;
			c_element->frame_thickness_ = 2.f;
		}

		utils::e_begin_layout(LayoutHorizontal, 4.f);
			utils::e_button(L"Create Sample Scene", [](void* c) {
				auto e = *(void**)c;
				looper().add_event([](void* c, bool*) {
					auto e_scene = *(Entity**)c;
					e_scene->remove_children(0, -1);

					auto e_box1 = Entity::create();
					e_scene->add_child(e_box1);
					{
						auto c_element = cElement::create();
						c_element->pos_ = 50.f;
						c_element->size_ = 400.f;
						c_element->color_ = Vec4c(255, 0, 0, 255);
						e_box1->add_component(c_element);
					}

					auto e_box2 = Entity::create();
					e_box1->add_child(e_box2);
					{
						auto c_element = cElement::create();
						c_element->pos_ = 50.f;
						c_element->size_ = 300.f;
						c_element->color_ = Vec4c(255, 255, 0, 255);
						e_box2->add_component(c_element);
					}

					auto e_text = Entity::create();
					e_box2->add_child(e_text);
					{
						auto c_element = cElement::create();
						c_element->pos_.x() = 12.f;
						c_element->pos_.y() = 8.f;
						e_text->add_component(c_element);

						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->color_ = Vec4c(0, 0, 0, 255);
						c_text->set_text(L"Hello World!");
						e_text->add_component(c_text);
					}

				}, Mail::from_p(e));
			}, Mail::from_p(e_scene));
			utils::e_button(L"Clear Scene", [](void* c) {
				auto e = *(void**)c;
				looper().add_event([](void* c, bool*) {
					(*(Entity**)c)->remove_children(0, -1);
				}, Mail::from_p(e));
			}, Mail::from_p(e_scene));
			utils::e_button(L"Save Scene", [](void* c) {
				auto e_scene = *(Entity**)c;
				if (e_scene->child_count() > 0)
					Entity::save_to_file(e_scene->child(0), L"test.prefab");
			}, Mail::from_p(e_scene));
			utils::e_button(L"Load Scene", [](void* c) {
				auto e = *(void**)c;
				looper().add_event([](void* c, bool*) {
					auto e_scene = *(Entity**)c;
					e_scene->remove_children(0, -1);
					if (std::filesystem::exists(L"test.prefab"))
						e_scene->add_child(Entity::create_from_file(e_scene->world(), L"test.prefab"));
				}, Mail::from_p(e));
			}, Mail::from_p(e_scene));
		utils::e_end_layout();
	utils::e_end_layout();

	utils::pop_parent();

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, Mail::from_p(&app));

	return 0;
}
