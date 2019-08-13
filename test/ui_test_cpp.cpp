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
#include <flame/universe/default_style.h>
#include <flame/universe/components/event_dispatcher.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>

using namespace flame;
using namespace graphics;

const auto img_id = 9;

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
	cElement* c_element_root;
	cText* c_text_fps;
	//wLayout* layout;

	void create_elements()
	{
		//ui->set_default_style(style);
		//if (style == DefaultStyleDark)
		//	canvas->set_clear_color(Vec4c(0, 0, 0, 255));
		//else
		//	canvas->set_clear_color(Vec4c(200, 200, 200, 255));

		//auto layout1 = Element::createT<wLayout>(ui, LayoutVertical);
		//layout1->pos$ = Vec2f(16.f, 8.f);
		//layout1->item_padding$ = 8.f;

		//auto w_checkbox = Element::createT<wCheckbox>(ui);
		//w_checkbox->align$ = AlignLittleEnd;
		//layout1->add_child(w_checkbox);

		//auto w_text = Element::createT<wText>(ui, font_atlas_index);
		//w_text->align$ = AlignLittleEnd;
		//w_text->text$ = L"some text";
		//w_text->set_size_auto();
		//layout1->add_child(w_text);

		//auto w_button = Element::createT<wButton>(ui, font_atlas_index, L"button");
		//w_button->align$ = AlignLittleEnd;
		//layout1->add_child(w_button);

		//auto w_toggle = Element::createT<wToggle>(ui, font_atlas_index);
		//w_toggle->align$ = AlignLittleEnd;
		//w_toggle->text$ = L"toggle";
		//w_toggle->set_size_auto();
		//w_toggle->set_toggle(true);
		//layout1->add_child(w_toggle);

		//auto w_menubar = Element::createT<wMenuBar>(ui);
		//w_menubar->align$ = AlignLittleEnd;

		//auto w_menu = Element::createT<wMenu>(ui, font_atlas_index, L"menu");

		//auto w_menuitem1 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 1");
		//auto w_menuitem2 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 2");
		//auto w_menuitem3 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 3");

		//w_menu->w_items()->add_child(w_menuitem1);
		//w_menu->w_items()->add_child(w_menuitem2);
		//w_menu->w_items()->add_child(w_menuitem3);

		//w_menubar->add_child(w_menu);

		//layout1->add_child(w_menubar);

		//auto w_combo = Element::createT<wCombo>(ui, font_atlas_index);
		//w_combo->align$ = AlignLittleEnd;

		//auto w_comboitem1 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 1");
		//auto w_comboitem2 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 2");
		//auto w_comboitem3 = Element::createT<wMenuItem>(ui, font_atlas_index, L"item 3");

		//w_combo->w_items()->add_child(w_comboitem1);
		//w_combo->w_items()->add_child(w_comboitem2);
		//w_combo->w_items()->add_child(w_comboitem3);

		//layout1->add_child(w_combo);

		//auto w_edit = Element::createT<wEdit>(ui, font_atlas_index);
		//w_edit->align$ = AlignLittleEnd;
		//w_edit->set_size_by_width(100.f);
		//layout1->add_child(w_edit);

		//auto w_image = Element::createT<wImage>(ui);
		//w_image->size$ = Vec2f(250.f);
		//w_image->id() = img_id;
		//w_image->align$ = AlignLittleEnd;
		//layout1->add_child(w_image);

		//layout->add_child(layout1, 1);

		//auto w_list = Element::createT<wList>(ui);
		//w_list->pos$ = Vec2f(800.f, 8.f);
		//w_list->size$ = Vec2f(300.f);

		//auto w_sizedrag = Element::createT<wSizeDrag>(ui, w_list);
		//w_sizedrag->min_size() = Vec2f(100.f);

		//w_list->add_child(w_sizedrag, 1);

		//for (auto i = 0; i < 20; i++)
		//{
		//	auto item = Element::createT<wListItem>(ui, font_atlas_index, (L"item " + to_stdwstring(i)).c_str());
		//	w_list->add_child(item);
		//}

		//layout->add_child(w_list, 1);

		//auto w_treenode1 = Element::createT<wTreeNode>(ui, font_atlas_index, L"A");
		//w_treenode1->pos$ = Vec2f(800.f, 400.f);

		//auto w_treenode2 = Element::createT<wTreeNode>(ui, font_atlas_index, L"B");
		//auto w_treenode3 = Element::createT<wTreeNode>(ui, font_atlas_index, L"C");
		//auto w_treenode4 = Element::createT<wTreeNode>(ui, font_atlas_index, L"D");

		//w_treenode1->w_items()->add_child(w_treenode2);
		//w_treenode1->w_items()->add_child(w_treenode3);
		//w_treenode3->w_items()->add_child(w_treenode4);

		//layout->add_child(w_treenode1, 1);

		//if (style == DefaultStyleDark)
		//	t_fps->text_col() = Vec4c(255, 255, 255, 255);
		//else if (style == DefaultStyleLight)
		//	t_fps->text_col() = Vec4c(0, 0, 0, 255);
	}

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

			c_element_root->width = w->size.x();
			c_element_root->height = w->size.y();

			c_text_fps->set_text(std::to_wstring(app_fps()));
			root->update();

			auto img_idx = sc->image_index();
			auto cb = cbs[img_idx];
			canvas->record(cb, img_idx);

			d->gq->submit(cb, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	typeinfo_load(L"flame_graphics.typeinfo");
	
	app.w = Window::create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
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
	{
		app.c_element_root = cElement::create(app.root, app.canvas);

		cEventDispatcher::create(app.root, app.w);

		cLayout::create(app.root);
	}

	auto e_fps = Entity::create();
	{
		cElement::create(e_fps);

		auto c_text = cText::create(e_fps, app.font_atlas1);
		c_text->color = Vec4c(0, 0, 0, 255);
		app.c_text_fps = c_text;

		auto c_aligner = cAligner::create(e_fps);
		c_aligner->x_align = AlignxLeft;
		c_aligner->y_align = AlignyBottom;
	}
	app.root->add_child(e_fps);

	app.canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.d, L"../asset/ui/imgs/9.png")));

	auto e_layout_top = Entity::create();
	{
		cElement::create(e_layout_top);

		auto c_layout = cLayout::create(e_layout_top);
		c_layout->type = LayoutHorizontal;

		auto c_aligner = cAligner::create(e_layout_top);
		c_aligner->x_align = AlignxMiddle;
		c_aligner->y_align = AlignyTop;
	}
	app.root->add_child(e_layout_top);

	auto e_btn_dark = Entity::create();
	{
		cElement::create(e_btn_dark);

		auto c_text = cText::create(e_btn_dark, app.font_atlas1);
		c_text->color = Vec4c(0, 0, 0, 255);
		c_text->set_text(L"Dark");

		cAligner::create(e_btn_dark);

		auto c_event_receiver = cEventReceiver::create(e_btn_dark);
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			if (is_mouse_clicked(action, key))
			{
				auto thiz = *(App * *)c;
				//	layout->clear_children(1, 0, -1, true);
				//	app->create_elements(DefaultStyleDark);
			}
		}, new_mail_p(&app));
	}
	e_layout_top->add_child(e_btn_dark);

	auto e_btn_light = Entity::create();
	{
		cElement::create(e_btn_light);

		auto c_text = cText::create(e_btn_light, app.font_atlas1);
		c_text->color = Vec4c(0, 0, 0, 255);
		c_text->set_text(L"Light");

		cAligner::create(e_btn_light);

		auto c_event_receiver = cEventReceiver::create(e_btn_light);
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			if (is_mouse_clicked(action, key))
			{
				auto thiz = *(App * *)c;
				//	layout->clear_children(1, 0, -1, true);
				//	app->create_elements(DefaultStyleLight);
			}
		}, new_mail_p(&app));
	}
	e_layout_top->add_child(e_btn_light);

	//layout = Element::createT<wLayout>(ui);
	//ui->root()->add_child(layout, 1);

	app.create_elements();

	app.cbs.resize(sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
