#include <flame/basic_app.h>

#include <flame/graphics/image.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/ui.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>

using namespace flame;
using namespace graphics;

const auto img_id = 59;
//wLayout* layout;

struct App;
typedef App* AppPtr;

struct App : BasicApp
{
	Canvas* canvas;
	Font* font_msyh;
	Font* font_awesome;
	FontAtlas* font_atlas;
	int font_atlas_index;

	Entity* root;

	//wText* t_fps;

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

	virtual void on_create() override
	{
		Canvas::initialize(d, sc);
		canvas = Canvas::create(sc);

		font_msyh = Font::create(L"c:/windows/fonts/msyh.ttc", 16);
		font_awesome = Font::create(L"../asset/font_awesome.ttf", 16);
		font_atlas = FontAtlas::create(d, 16, false, { font_msyh, font_awesome });
		font_atlas_index = canvas->add_font_atlas(font_atlas);

		root = Entity::create();
		auto ui = cUI$::create$(nullptr);
		ui->set_canvas(canvas);
		root->add_component(ui);

		auto bg = Entity::create();
		auto wBackground = cElement$::create$(nullptr);
		wBackground->size = Vec2f(100.f);
		wBackground->background_color = Vec4c(255, 128, 128, 255);
		bg->add_component(wBackground);

		auto wFps = cText$::create$(nullptr);
		wFps->font_atlas_index = font_atlas_index;
		wFps->set_text(L"QAQ");
		bg->add_component(wFps);

		root->add_child(bg);

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
	}

	virtual void do_run() override
	{
		sc->acquire_image(image_avalible);

		//t_fps->text$ = L"FPS:" + std::to_wstring(app->fps);
		//t_fps->set_size_auto();

		root->update(app->elapsed_time);

		canvas->record_cb();

		d->gq->submit(canvas->get_cb(), image_avalible, render_finished);
		d->gq->wait_idle();

		d->gq->present(sc, render_finished);
	}
}app;

int main(int argc, char** args)
{
	Ivec2 res(1280, 720);

	app.create("UI Test", res, WindowFrame);
	app.run();

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
