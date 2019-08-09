#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/ui.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>

using namespace flame;
using namespace graphics;

const auto img_id = 59;
//wLayout* layout;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;

	FontAtlas* font_atlas1;
	FontAtlas* font_atlas2;
	Canvas* canvas;
	int rt_frame;

	Entity* root;

	//void create_elements(DefaultStyle style)
	//{
	//	ui->set_default_style(style);
	//	if (style == DefaultStyleDark)
	//		canvas->set_clear_color(Vec4c(0, 0, 0, 255));
	//	else
	//		canvas->set_clear_color(Vec4c(200, 200, 200, 255));

	//	auto layout1 = Element::createT<wLayout>(ui, LayoutVertical);
	//	layout1->pos$ = Vec2f(16.f, 8.f);
	//	layout1->item_padding$ = 8.f;

	//	auto w_checkbox = Element::createT<wCheckbox>(ui);
	//	w_checkbox->align$ = AlignLittleEnd;
	//	layout1->add_child(w_checkbox);

	//	auto w_text = Element::createT<wText>(ui, font_atlas_index);
	//	w_text->align$ = AlignLittleEnd;
	//	w_text->text$ = L"some text";
	//	w_text->set_size_auto();
	//	layout1->add_child(w_text);

	//	auto w_button = Element::createT<wButton>(ui, font_atlas_index, L"button");
	//	w_button->align$ = AlignLittleEnd;
	//	layout1->add_child(w_button);

	//	auto w_toggle = Element::createT<wToggle>(ui, font_atlas_index);
	//	w_toggle->align$ = AlignLittleEnd;
	//	w_toggle->text$ = L"toggle";
	//	w_toggle->set_size_auto();
	//	w_toggle->set_toggle(true);
	//	layout1->add_child(w_toggle);

	//	auto w_menubar = Element::createT<wMenuBar>(ui);
	//	w_menubar->align$ = AlignLittleEnd;

	//	auto w_menu = Element::createT<wMenu>(ui, font_atlas_index, L"menu");

	//	auto w_menuitem1 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 1");
	//	auto w_menuitem2 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 2");
	//	auto w_menuitem3 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 3");

	//	w_menu->w_items()->add_child(w_menuitem1);
	//	w_menu->w_items()->add_child(w_menuitem2);
	//	w_menu->w_items()->add_child(w_menuitem3);

	//	w_menubar->add_child(w_menu);

	//	layout1->add_child(w_menubar);

	//	auto w_combo = Element::createT<wCombo>(ui, font_atlas_index);
	//	w_combo->align$ = AlignLittleEnd;

	//	auto w_comboitem1 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 1");
	//	auto w_comboitem2 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 2");
	//	auto w_comboitem3 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 3");

	//	w_combo->w_items()->add_child(w_comboitem1);
	//	w_combo->w_items()->add_child(w_comboitem2);
	//	w_combo->w_items()->add_child(w_comboitem3);

	//	layout1->add_child(w_combo);

	//	auto w_edit = Element::createT<wEdit>(ui, font_atlas_index);
	//	w_edit->align$ = AlignLittleEnd;
	//	w_edit->set_size_by_width(100.f);
	//	layout1->add_child(w_edit);

	//	auto w_image = Element::createT<wImage>(ui);
	//	w_image->size$ = Vec2f(250.f);
	//	w_image->id() = img_id;
	//	w_image->align$ = AlignLittleEnd;
	//	layout1->add_child(w_image);

	//	layout->add_child(layout1, 1);

	//	auto w_list = Element::createT<wList>(ui);
	//	w_list->pos$ = Vec2f(800.f, 8.f);
	//	w_list->size$ = Vec2f(300.f);

	//	auto w_sizedrag = Element::createT<wSizeDrag>(ui, w_list);
	//	w_sizedrag->min_size() = Vec2f(100.f);

	//	w_list->add_child(w_sizedrag, 1);

	//	for (auto i = 0; i < 20; i++)
	//	{
	//		auto item = Element::createT<wListItem>(ui, font_atlas_index, (L"item " + to_stdwstring(i)).c_str());
	//		w_list->add_child(item);
	//	}

	//	layout->add_child(w_list, 1);

	//	auto w_treenode1 = Element::createT<wTreeNode>(ui, font_atlas_index, L"A");
	//	w_treenode1->pos$ = Vec2f(800.f, 400.f);

	//	auto w_treenode2 = Element::createT<wTreeNode>(ui, font_atlas_index, L"B");
	//	auto w_treenode3 = Element::createT<wTreeNode>(ui, font_atlas_index, L"C");
	//	auto w_treenode4 = Element::createT<wTreeNode>(ui, font_atlas_index, L"D");

	//	w_treenode1->w_items()->add_child(w_treenode2);
	//	w_treenode1->w_items()->add_child(w_treenode3);
	//	w_treenode3->w_items()->add_child(w_treenode4);

	//	layout->add_child(w_treenode1, 1);

	//	if (style == DefaultStyleDark)
	//		t_fps->text_col() = Vec4c(255, 255, 255, 255);
	//	else if (style == DefaultStyleLight)
	//		t_fps->text_col() = Vec4c(0, 0, 0, 255);
	//}

	void run()
	{
		auto sc = scr->sc();
		auto sc_frame = scr->sc_frame();

		if (sc_frame > rt_frame)
		{
			canvas->set_render_target(TargetImages, sc ? &sc->images() : nullptr);
			rt_frame = sc_frame;
		}

		if (sc)
		{
			sc->acquire_image();
			fence->wait();

			root->update();

			auto img_idx = sc->image_index();
			auto cb = cbs[img_idx];
			canvas->record(cb, img_idx);

			d->gq->submit(cb, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;
auto papp = &app;

int main(int argc, char** args)
{
	typeinfo_load(L"flame_graphics.typeinfo");
	
	app.w = Window::create("UI Test", Vec2u(1280, 720), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);

	auto sc = app.scr->sc();

	app.canvas = Canvas::create(app.d, TargetImages, &sc->images());

	auto font_msyh = Font::create(L"c:/windows/fonts/consola.ttf", 14);
	auto font_awesome = Font::create(L"../asset/font_awesome.ttf", 14);
	app.font_atlas1 = FontAtlas::create(app.d, FontDrawPixel, { font_msyh, font_awesome });
	app.font_atlas2 = FontAtlas::create(app.d, FontDrawSdf, { font_msyh });
	auto font_atlas_view1 = Imageview::create(app.font_atlas1->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR);
	auto font_atlas_view2 = Imageview::create(app.font_atlas2->image());
	app.font_atlas1->index = 1;
	app.font_atlas2->index = 2;
	app.canvas->set_image(app.font_atlas1->index, font_atlas_view1);
	app.canvas->set_image(app.font_atlas2->index, font_atlas_view2);

	app.root = Entity::create();
	auto c_ui = cUI::create(app.canvas, app.w);
	app.root->add_component(c_ui);

	auto e_fps = Entity::create();
	app.root->add_child(e_fps);
	auto c_element = cElement::create();
	e_fps->add_component(c_element);
	auto c_text = cText::create();
	c_text->font_atlas = app.font_atlas1;
	c_text->color = Vec4c(0, 0, 0, 255);
	c_text->set_text(L"QAQ");
	e_fps->add_component(c_text);

	//t_fps = Element::createT<wText>(ui, font_atlas_index);
	//t_fps->align$ = AlignLeftBottom;
	//ui->root()->add_child(t_fps, 1);

	//auto img = Image::create_from_file(d, L"../asset/ui/imgs/9.png");
	//canvas->set_imageview(img_id, Imageview::get(img));

	//auto layout_top = Element::createT<wLayout>(ui, LayoutHorizontal);
	//layout_top->align$ = AlignTop;

	//auto w_btn_dark = Element::createT<wButton>(ui, font_atlas_index, L"dark");
	//w_btn_dark->align$ = AlignLittleEnd;
	//w_btn_dark->text_col() = Vec4c(255);
	//w_btn_dark->mouse_listeners$.push_back(Function<Element::MouseListenerParm>([](Element::MouseListenerParm &p) {
	//	auto app = p.get_capture<AppData>().app();
	//	if (!p.is_clicked())
	//		return;

	//	layout->clear_children(1, 0, -1, true);
	//	app->create_elements(DefaultStyleDark);
	//}, { this }));
	//layout_top->add_child(w_btn_dark);

	//auto w_btn_light = Element::createT<wButton>(ui, font_atlas_index, L"light");
	//w_btn_light->align$ = AlignLittleEnd;
	//w_btn_light->text_col() = Vec4c(255);
	//w_btn_light->mouse_listeners$.push_back(Function<Element::MouseListenerParm>([](Element::MouseListenerParm & p) {
	//	auto app = p.get_capture<AppData>().app();
	//	if (!p.is_clicked())
	//		return;

	//	layout->clear_children(1, 0, -1, true);
	//	app->create_elements(DefaultStyleLight);
	//}, { this }));
	//layout_top->add_child(w_btn_light);

	//ui->root()->add_child(layout_top, 1);

	//layout = Element::createT<wLayout>(ui);
	//ui->root()->add_child(layout, 1);
	//create_elements(DefaultStyleDark);

	app.cbs.resize(sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}

//{
//	auto file = SerializableNode::create("ui");

//	auto n = w_combo->save();
//	file->add_node(n);

//	file->save_xml(L"d:/ui.xml");
//	SerializableNode::destroy(file);
//}
//{
//	auto file = SerializableNode::create_from_xml(L"d:/ui.xml");
//	if (file)
//	{
//		auto w = Element::create_from_file(ui, file->node(0));

//		SerializableNode::destroy(file);

//		layout1->add_child(w);
//	}
//}
