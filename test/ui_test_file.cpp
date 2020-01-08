#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

#include "../renderpath/canvas/canvas.h"

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	std::vector<TypeinfoDatabase*> dbs;

	FontAtlas* font_atlas_pixel;

	Universe* u;
	cElement* c_element_root;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		if (sc)
		{
			c_element_root->set_size(Vec2f(w->size));
			u->update();
		}

		if (sc)
		{
			d->gq->submit(1, &cbs.v[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	app.w = Window::create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->image_count());
	for (auto i = 0; i < app.cbs.s; i++)
		app.cbs.v[i] = Commandbuffer::create(app.d->gcp);
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs.size(), app.dbs.data(), L"flame_foundation.typeinfo"));
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs.size(), app.dbs.data(), L"flame_graphics.typeinfo"));
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs.size(), app.dbs.data(), L"flame_universe.typeinfo"));

	app.u = Universe::create();
	app.u->add_object(app.w);

	auto w = World::create(app.u);
	w->add_system(sLayoutManagement::create());
	w->add_system(sEventDispatcher::create());
	auto s_2d_renderer = s2DRenderer::create(L"../renderpath/canvas/bp", app.scr, FLAME_CHASH("SwapchainResizable"), &app.cbs);
	w->add_system(s_2d_renderer);
	{
		auto canvas = s_2d_renderer->canvas;
		wchar_t* fonts[] = {
			L"c:/windows/fonts/msyh.ttc"
		};

		app.font_atlas_pixel = FontAtlas::create(app.d, FontDrawPixel, 1, fonts);
		canvas->add_font(app.font_atlas_pixel);

		w->add_object(app.font_atlas_pixel);
	}

	auto root = w->root();
	{
		app.c_element_root = cElement::create();
		root->add_component(app.c_element_root);

		root->add_component(cLayout::create(LayoutFree));
	}

	auto e_fps = Entity::create();
	root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_pixel);
		e_fps->add_component(c_text);

		auto c_aligner = cAligner::create();
		c_aligner->x_align_ = AlignxLeft;
		c_aligner->y_align_ = AlignyBottom;
		e_fps->add_component(c_aligner);

		add_fps_listener([](void* c, uint fps) {
			(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
		}, new_mail_p(c_text));
	}

	auto e_layout = Entity::create();
	root->add_child(e_layout);
	{
		auto c_element = cElement::create();
		c_element->pos_.x() = 16.f;
		c_element->pos_.y() = 28.f;
		e_layout->add_component(c_element);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 16.f;
		e_layout->add_component(c_layout);
	}

	auto e_scene = Entity::create();
	e_layout->add_child(e_scene);
	{
		auto c_element = cElement::create();
		c_element->size_ = 500.f;
		c_element->frame_thickness_ = 2.f;
		e_scene->add_component(c_element);
	}

	auto e_buttons = Entity::create();
	e_layout->add_child(e_buttons);
	{
		e_buttons->add_component(cElement::create());

		auto c_layout = cLayout::create(LayoutHorizontal);
		c_layout->item_padding = 4.f;
		e_buttons->add_component(c_layout);
	}

	auto e_btn_create_sample_scene = create_standard_button(app.font_atlas_pixel, 1.f, L"Create Sample Scene");
	e_buttons->add_child(e_btn_create_sample_scene);
	{
		e_btn_create_sample_scene->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto e_scene = *(Entity**)c;
			if (is_mouse_clicked(action, key))
			{
				looper().add_event([](void* c) {
					auto e_scene = *(Entity**)c;

					e_scene->remove_child((Entity*)FLAME_INVALID_POINTER);

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
						c_text->color = Vec4c(0, 0, 0, 255);
						c_text->set_text(L"Hello World!");
						e_text->add_component(c_text);
					}

				}, new_mail_p(e_scene));
			}
		}, new_mail_p(e_scene));
	}

	auto e_btn_clear_scene = create_standard_button(app.font_atlas_pixel, 1.f, L"Clear Scene");
	e_buttons->add_child(e_btn_clear_scene);
	{
		e_btn_clear_scene->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto e_scene = *(Entity**)c;
			if (is_mouse_clicked(action, key))
			{
				looper().add_event([](void* c) {
					auto e_scene = *(Entity**)c;

					e_scene->remove_child((Entity*)FLAME_INVALID_POINTER);
				}, new_mail_p(e_scene));
			}
		}, new_mail_p(e_scene));
	}

	auto e_btn_save_scene = create_standard_button(app.font_atlas_pixel, 1.f, L"Save Scene");
	e_buttons->add_child(e_btn_save_scene);
	{
		e_btn_save_scene->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto e_scene = *(Entity**)c;
			if (is_mouse_clicked(action, key))
			{
				looper().add_event([](void* c) {
					auto e_scene = *(Entity**)c;

					if (e_scene->child_count() > 0)
						Entity::save_to_file(app.dbs.size(), app.dbs.data(), e_scene->child(0), L"test.prefab");
				}, new_mail_p(e_scene));
			}
		}, new_mail_p(e_scene));
	}

	auto e_btn_load_scene = create_standard_button(app.font_atlas_pixel, 1.f, L"Load Scene");
	e_buttons->add_child(e_btn_load_scene);
	{
		e_btn_load_scene->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			auto e_scene = *(Entity**)c;
			if (is_mouse_clicked(action, key))
			{
				looper().add_event([](void* c) {
					auto e_scene = *(Entity**)c;

					e_scene->remove_child((Entity*)FLAME_INVALID_POINTER);
					if (std::filesystem::exists(L"test.prefab"))
						e_scene->add_child(Entity::create_from_file(e_scene->world_, app.dbs.size(), app.dbs.data(), L"test.prefab"));
				}, new_mail_p(e_scene));
			}
		}, new_mail_p(e_scene));
	}

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
