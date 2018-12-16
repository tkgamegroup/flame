// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/file.h>
#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/ui/style.h>

#include <flame/basic_app.h>

using namespace flame;

BasicApp app;

const auto img_id = 99;
ui::wLayout *layout;

void create_widgets(ui::DefaultStyle style)
{
	using namespace ui;

	app.ui_ins->set_default_style(style);
	if (style == DefaultStyleDark)
		app.canvas->set_clear_color(Bvec4(0, 0, 0, 255));
	else
		app.canvas->set_clear_color(Bvec4(200, 200, 200, 255));

	auto layout1 = Widget::createT<wLayout>(app.ui_ins, LayoutVertical);
	layout1->pos$ = Vec2(16.f, 8.f);
	layout1->item_padding$ = 8.f;

	auto w_checkbox = Widget::createT<wCheckbox>(app.ui_ins);
	w_checkbox->align$ = AlignLittleEnd;
	layout1->add_child(w_checkbox);

	auto w_text = Widget::createT<wText>(app.ui_ins);
	w_text->align$ = AlignLittleEnd;
	w_text->text() = L"sub-pixel";
	w_text->set_size_auto();
	layout1->add_child(w_text);

	auto w_text_sdf = Widget::createT<wText>(app.ui_ins);
	w_text_sdf->align$ = AlignLittleEnd;
	w_text_sdf->sdf_scale() = 4.f;
	w_text_sdf->text() = L"SDF";
	w_text_sdf->set_size_auto();
	layout1->add_child(w_text_sdf);

	auto w_button = Widget::createT<wButton>(app.ui_ins, L"button");
	w_button->align$ = AlignLittleEnd;
	layout1->add_child(w_button);

	auto w_toggle1 = Widget::createT<wToggle>(app.ui_ins);
	w_toggle1->align$ = AlignLittleEnd;
	w_toggle1->text() = L"toggled";
	w_toggle1->set_size_auto();
	w_toggle1->set_toggle(true);
	layout1->add_child(w_toggle1);

	auto w_toggle2 = Widget::createT<wToggle>(app.ui_ins);
	w_toggle2->align$ = AlignLittleEnd;
	w_toggle2->text() = L"untoggled";
	w_toggle2->set_size_auto();
	w_toggle2->set_toggle(false);
	layout1->add_child(w_toggle2);

	auto w_menubar = Widget::createT<wMenuBar>(app.ui_ins);
	w_menubar->align$ = AlignLittleEnd;

	auto w_menu = Widget::createT<wMenu>(app.ui_ins, L"menu");

	auto w_menuitem1 = Widget::createT<wMenuItem>(app.ui_ins, L"item 1");
	auto w_menuitem2 = Widget::createT<wMenuItem>(app.ui_ins, L"item 2");
	auto w_menuitem3 = Widget::createT<wMenuItem>(app.ui_ins, L"item 3");

	w_menu->w_items()->add_child(w_menuitem1);
	w_menu->w_items()->add_child(w_menuitem2);
	w_menu->w_items()->add_child(w_menuitem3);

	w_menubar->add_child(w_menu);

	layout1->add_child(w_menubar);

	//{
	//	auto file = SerializableNode::create_from_xml(L"d:/ui.xml");
	//	if (file)
	//	{
	//		auto w = Widget::create_from_file(app.ui_ins, file->node(0));

	//		SerializableNode::destroy(file);

	//		layout1->add_child(w);
	//	}
	//}
	auto w_combo = Widget::createT<wCombo>(app.ui_ins);
	w_combo->align$ = AlignLittleEnd;
	//add_style_background_color(w_combo, 0, Vec3(0.f, 0.f, 0.7f));

	auto w_comboitem1 = Widget::createT<wMenuItem>(app.ui_ins, L"item 1");
	auto w_comboitem2 = Widget::createT<wMenuItem>(app.ui_ins, L"item 2");
	auto w_comboitem3 = Widget::createT<wMenuItem>(app.ui_ins, L"item 3");

	w_combo->w_items()->add_child(w_comboitem1);
	w_combo->w_items()->add_child(w_comboitem2);
	w_combo->w_items()->add_child(w_comboitem3);

	layout1->add_child(w_combo);

	auto w_edit = Widget::createT<wEdit>(app.ui_ins);
	w_edit->align$ = AlignLittleEnd;
	w_edit->set_size_by_width(100.f);
	layout1->add_child(w_edit);

	auto w_image = Widget::createT<wImage>(app.ui_ins);
	w_image->size$ = Vec2(250.f);
	w_image->id() = img_id;
	w_image->align$ = AlignLittleEnd;
	layout1->add_child(w_image);

	layout->add_child(layout1, 1, -1, true);

	auto w_list = Widget::createT<wList>(app.ui_ins);
	w_list->pos$ = Vec2(800.f, 8.f);
	w_list->size$ = Vec2(300.f);

	auto w_sizedrag = Widget::createT<wSizeDrag>(app.ui_ins, w_list);
	w_sizedrag->min_size() = Vec2(100.f);

	w_list->add_child(w_sizedrag, 1);

	for (auto i = 0; i < 20; i++)
	{
		auto item = Widget::createT<wListItem>(app.ui_ins, (L"item " + to_stdwstring(i)).c_str());
		w_list->add_child(item);
	}

	layout->add_child(w_list, 1, -1, true);

	auto w_treenode1 = Widget::createT<wTreeNode>(app.ui_ins, L"A");
	w_treenode1->pos$ = Vec2(800.f, 400.f);

	auto w_treenode2 = Widget::createT<wTreeNode>(app.ui_ins, L"B");
	auto w_treenode3 = Widget::createT<wTreeNode>(app.ui_ins, L"C");
	auto w_treenode4 = Widget::createT<wTreeNode>(app.ui_ins, L"D");

	w_treenode1->w_items()->add_child(w_treenode2);
	w_treenode1->w_items()->add_child(w_treenode3);
	w_treenode3->w_items()->add_child(w_treenode4);

	layout->add_child(w_treenode1, 1, -1, true);

	if (style == DefaultStyleDark)
		app.t_fps->text_col() = Bvec4(255, 255, 255, 255);
	else if (style == DefaultStyleLight)
		app.t_fps->text_col() = Bvec4(0, 0, 0, 255);
}

extern "C" __declspec(dllexport) int main()
{
	using namespace ui;

	app.create("ui test", Ivec2(1280, 720), WindowFrame | WindowResizable);

	auto img = graphics::Image::create_from_file(app.d, L"ui/imgs/9.png");
	app.ui_ins->set_imageview(img_id, graphics::Imageview::get(img));

	auto layout_top = Widget::createT<wLayout>(app.ui_ins, LayoutHorizontal);
	layout_top->align$ = AlignTop;

	auto w_btn_dark = Widget::createT<wButton>(app.ui_ins, L"dark");
	w_btn_dark->align$ = AlignLittleEnd;
	w_btn_dark->text_col() = Bvec4(255);
	w_btn_dark->add_listener(Widget::ListenerMouse, [](const ParmPackage &_p) {
		auto &p = (Widget::MouseListenerParm&)_p;
		if (!(p.action() == (KeyStateDown | KeyStateUp) && p.key() == Mouse_Null))
			return;

		layout->clear_children(1, 0, -1, true);
		create_widgets(DefaultStyleDark);
	}, nullptr, {});
	layout_top->add_child(w_btn_dark);
	auto w_btn_light = Widget::createT<wButton>(app.ui_ins, L"light");
	w_btn_light->align$ = AlignLittleEnd;
	w_btn_light->text_col() = Bvec4(255);
	w_btn_light->add_listener(Widget::ListenerMouse, [](const ParmPackage &_p) {
		auto &p = (Widget::MouseListenerParm&)_p;
		if (!(p.action() == (KeyStateDown | KeyStateUp) && p.key() == Mouse_Null))
			return;

		layout->clear_children(1, 0, -1, true);
		create_widgets(DefaultStyleLight);
	}, nullptr, {});
	layout_top->add_child(w_btn_light);

	app.ui_ins->root()->add_child(layout_top, 1);

	layout = Widget::createT<wLayout>(app.ui_ins);
	app.ui_ins->root()->add_child(layout, 1);
	create_widgets(DefaultStyleDark);
	//{
	//	auto file = SerializableNode::create("ui");

	//	auto n = w_combo->save();
	//	file->add_node(n);

	//	file->save_xml(L"d:/ui.xml");
	//	SerializableNode::destroy(file);
	//}

	app.run();

	return 0;
}
