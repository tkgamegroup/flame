#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct App
{
	Window* w;
	Device* d;
	SwapchainResizable* scr;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	Semaphore* render_finished;

	FontAtlas* font_atlas_pixel;
	FontAtlas* font_atlas_lcd;
	FontAtlas* font_atlas_sdf;

	Universe* u;
	Entity* root;
	cElement* c_element_root;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		c_element_root->set_size(Vec2f(w->size));
		u->update();

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
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->image_count());
	for (auto i = 0; i < app.cbs.s; i++)
		app.cbs.v[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);

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
			L"c:/windows/fonts/msyh.ttc",
			L"../art/font_awesome.ttf"
		};
		app.font_atlas_pixel = FontAtlas::create(app.d, FontDrawPixel, 2, fonts);
		canvas->add_font(app.font_atlas_pixel);
		app.font_atlas_lcd = FontAtlas::create(app.d, FontDrawLcd, 1, fonts);
		canvas->add_font(app.font_atlas_lcd);
		app.font_atlas_sdf = FontAtlas::create(app.d, FontDrawSdf, 1, fonts);
		canvas->add_font(app.font_atlas_sdf);

		canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.d, L"../art/ui/imgs/9.png")));
	}

	auto root = w->root();
	app.root = root;

	ui::set_current_entity(root);
	app.c_element_root = ui::c_element();
	ui::c_event_receiver();
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::push_parent(root);
	ui::set_current_root(root);

	ui::e_text(L"");
	ui::c_aligner(AlignxLeft, AlignyBottom);
	add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, new_mail_p(ui::current_entity()->get_component(cText)));

	ui::e_begin_menu_bar();
	ui::e_begin_menubar_menu(L"File");
	ui::e_menu_item(L"New", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Open", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_end_menubar_menu();
	ui::e_begin_menubar_menu(L"Edit");
	ui::e_menu_item(L"Undo", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Redo", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Cut", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Copy", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Paste", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Delete", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_end_menubar_menu();
	ui::e_begin_menubar_menu(L"Tool");
	ui::e_menu_item(L"Monitor", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Console", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_end_menubar_menu();
	ui::e_end_menu_bar();

	ui::e_begin_layout(Vec2f(16.f, 28.f), LayoutVertical, 16.f);

	ui::e_text(L"Text Pixel");
	ui::push_font_atlas(app.font_atlas_lcd);
	ui::e_text(L"Text Lcd");
	ui::pop_font_atlas();
	ui::push_font_atlas(app.font_atlas_sdf);
	ui::e_text(L"Text Sdf");
	ui::pop_font_atlas();
	ui::e_button(L"Click Me£¡", [](void* c, Entity* e) {
		e->get_component(cText)->set_text(L"Click Me! :)");
		printf("thank you for clicking me\n");
	}, Mail<>());
	ui::e_checkbox(L"Checkbox");
	ui::e_toggle(L"Toggle");
	ui::e_image(img_id << 16, Vec2f(250.f), 4.f, 2.f, Vec4c(10, 200, 10, 255));
	ui::e_edit(100.f);

	ui::e_end_layout();

	ui::e_begin_layout(Vec2f(416.f, 28.f), LayoutVertical, 16.f);

	ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(200.f, 100.f), 4.f, 2.f)->get_component(cElement);
	ui::e_begin_list(true);
	for (auto i = 0; i < 10; i++)
		ui::e_list_item((L"item" + std::to_wstring(i)).c_str());
	ui::e_end_list();
	ui::e_end_scroll_view1();

	ui::e_begin_tree(false, 4.f, 2.f);
	ui::e_begin_tree_node(L"A");
	ui::e_tree_leaf(L"C");
	ui::e_tree_leaf(L"D");
	ui::e_end_tree_node();
	ui::e_tree_leaf(L"B");
	ui::e_end_tree();

	ui::e_begin_combobox(100.f);
	ui::e_combobox_item(L"Apple");
	ui::e_combobox_item(L"Boy");
	ui::e_combobox_item(L"Cat");
	ui::e_end_combobox();

	ui::e_end_layout();

	ui::e_begin_docker_container(Vec2f(416.f, 300.f), Vec2f(200.f));
	ui::e_begin_docker();
	ui::e_begin_docker_page(L"ResourceExplorer");
	ui::e_text(L"flower.png  main.cpp");
	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_container();

	ui::e_begin_docker_container(Vec2f(640.f, 300.f), Vec2f(400.f, 200.f));
	ui::e_begin_docker_layout(LayoutHorizontal);
	ui::e_begin_docker();
	ui::e_begin_docker_page(L"TextEditor");
	ui::e_text(L"printf(\"Hello World!\\n\");");
	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_begin_docker_layout(LayoutVertical);
	ui::e_begin_docker();
	ui::e_begin_docker_page(L"Hierarchy");
	ui::e_text(L"Node A\n--Node B");
	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_begin_docker();
	ui::e_begin_docker_page(L"Inspector");
	ui::e_text(L"Name: James Bond\nID: 007");
	ui::e_end_docker_page();
	ui::e_end_docker();

	ui::e_end_docker_layout();

	ui::e_end_docker_layout();

	ui::e_end_docker_container();

	ui::e_begin_popup_menu();
	ui::e_menu_item(L"Refresh", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Save", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Help", [](void* c, Entity* e) {
		wprintf(L"%s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_begin_sub_menu(L"add");
	ui::e_menu_item(L"Tree", [](void* c, Entity* e) {
		wprintf(L"Add %s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Car", [](void* c, Entity* e) {
		wprintf(L"Add %s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"House", [](void* c, Entity* e) {
		wprintf(L"Add %s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_end_sub_menu();
	ui::e_begin_sub_menu(L"Remove");
	ui::e_menu_item(L"Tree", [](void* c, Entity* e) {
		wprintf(L"Remove %s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"Car", [](void* c, Entity* e) {
		wprintf(L"Remove %s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_menu_item(L"House", [](void* c, Entity* e) {
		wprintf(L"Remove %s!\n", e->get_component(cText)->text());
	}, Mail<>());
	ui::e_end_sub_menu();
	ui::e_end_popup_menu();

	ui::pop_parent();
	ui::pop_font_atlas();

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
