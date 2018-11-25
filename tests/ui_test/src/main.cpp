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
#include <flame/window.h>
#include <flame/typeinfo.h>
#include <flame/serialize.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/ui/instance.h>
#include <flame/ui/canvas.h>
#include <flame/ui/widget.h>
#include <flame/ui/style.h>

using namespace flame;

Application *app;
Window *w;
graphics::Device *d;
graphics::Swapchain *sc;
graphics::Semaphore *image_avalible;
graphics::Semaphore *ui_finished;
ui::SwapchainData *sd;
ui::Instance *ui_ins;
ui::Canvas *canvas;
ui::wText *t_fps;

static void do_run(CommonData *)
{
	auto index = sc->acquire_image(image_avalible);

	ui_ins->begin(app->elapsed_time);
	ui_ins->end(canvas);

	canvas->record_cb(index);

	d->gq->submit(canvas->get_cb(), image_avalible, ui_finished);
	d->gq->wait_idle();
	d->gq->present(index, sc, ui_finished);

	static wchar_t buf[16];
	swprintf(buf, L"FPS:%lld", app->fps);
	t_fps->set_text_and_size(buf);
}

extern "C" __declspec(dllexport) int main()
{
	typeinfo::load(L"typeinfo.xml");

	auto res = Ivec2(1280, 720);
	const auto img_id = 99;

	app = Application::create();
	w = Window::create(app, "ui Test", res, WindowFrame | WindowResizable);
	d = graphics::Device::get_shared();
	sc = graphics::Swapchain::create(d, w);
	image_avalible = graphics::Semaphore::create(d);
	ui_finished = graphics::Semaphore::create(d);
	sd = ui::SwapchainData::create(sc);
	ui_ins = ui::Instance::create(w);
	canvas = ui::Canvas::create(sd);

	auto img = graphics::Image::create_from_file(d, L"ui/imgs/9.png");
	ui_ins->set_imageview(img_id, graphics::Imageview::get(img));

	auto layout1 = ui::wLayout::create(ui_ins);
	layout1->pos$ = Vec2(16.f, 8.f);
	layout1->item_padding$ = 8.f;
	layout1->layout_type$ = ui::LayoutVertical;

	auto w_checkbox = ui::wCheckbox::create(ui_ins);
	w_checkbox->align$ = ui::AlignLittleEnd;
	layout1->add_child(w_checkbox);

	auto w_text = ui::wText::create(ui_ins);
	w_text->align$ = ui::AlignLittleEnd;
	w_text->text_col() = Bvec4(255);
	w_text->set_text_and_size(L"sub-pixel");
	layout1->add_child(w_text);

	auto w_text_sdf = ui::wText::create(ui_ins);
	w_text_sdf->align$ = ui::AlignLittleEnd;
	w_text_sdf->text_col() = Bvec4(255);
	w_text_sdf->sdf_scale() = 4.f;
	w_text_sdf->set_text_and_size(L"SDF");
	layout1->add_child(w_text_sdf);

	auto w_button = ui::wButton::create(ui_ins);
	w_button->align$ = ui::AlignLittleEnd;
	w_button->text_col() = Bvec4(255);
	w_button->set_classic(L"button");
	layout1->add_child(w_button);

	auto w_toggle1 = ui::wToggle::create(ui_ins);
	w_toggle1->align$ = ui::AlignLittleEnd;
	w_toggle1->text_col() = Bvec4(255);
	w_toggle1->set_text_and_size(L"toggled");
	w_toggle1->set_toggle(true);
	ui::add_style_color(w_toggle1, 0, Vec3(0.f, 0.f, 0.7f));
	ui::add_style_color(w_toggle1, 1, Vec3(150.f, 1.f, 0.8f));
	layout1->add_child(w_toggle1);

	auto w_toggle2 = ui::wToggle::create(ui_ins);
	w_toggle2->align$ = ui::AlignLittleEnd;
	w_toggle2->text_col() = Bvec4(255);
	w_toggle2->set_text_and_size(L"untoggled");
	w_toggle2->set_toggle(false);
	ui::add_style_color(w_toggle2, 0, Vec3(0.f, 0.f, 0.7f));
	ui::add_style_color(w_toggle2, 1, Vec3(150.f, 1.f, 0.8f));
	layout1->add_child(w_toggle2);

	auto w_menubar = ui::wMenuBar::create(ui_ins);
	w_menubar->align$ = ui::AlignLittleEnd;

	auto w_menu = ui::wMenu::create(ui_ins, L"menu");

	auto w_menuitem1 = ui::wMenuItem::create(ui_ins, L"item 1");
	auto w_menuitem2 = ui::wMenuItem::create(ui_ins, L"item 2");
	auto w_menuitem3 = ui::wMenuItem::create(ui_ins, L"item 3");

	w_menu->w_items()->add_child(w_menuitem1);
	w_menu->w_items()->add_child(w_menuitem2);
	w_menu->w_items()->add_child(w_menuitem3);

	w_menubar->add_child(w_menu);

	layout1->add_child(w_menubar);

	auto w_combo = ui::wCombo::create(ui_ins);
	w_combo->align$ = ui::AlignLittleEnd;
	ui::add_style_color(w_combo, 0, Vec3(0.f, 0.f, 0.7f));

	auto w_comboitem1 = ui::wMenuItem::create(ui_ins, L"item 1");
	auto w_comboitem2 = ui::wMenuItem::create(ui_ins, L"item 2");
	auto w_comboitem3 = ui::wMenuItem::create(ui_ins, L"item 3");

	w_combo->w_items()->add_child(w_comboitem1);
	w_combo->w_items()->add_child(w_comboitem2);
	w_combo->w_items()->add_child(w_comboitem3);

	layout1->add_child(w_combo);

	auto w_edit = ui::wEdit::create(ui_ins);
	w_edit->align$ = ui::AlignLittleEnd;
	w_edit->text_col() = Bvec4(255);
	w_edit->set_size_by_width(100.f);
	layout1->add_child(w_edit);

	auto w_image = ui::wImage::create(ui_ins);
	w_image->size$ = Vec2(250.f);
	w_image->id() = img_id;
	w_image->align$ = ui::AlignLittleEnd;
	layout1->add_child(w_image);

	ui_ins->root()->add_child(layout1, 1);

	auto w_list = ui::wList::create(ui_ins);
	w_list->pos$ = Vec2(300.f, 8.f);
	w_list->size$ = Vec2(300.f);

	auto w_sizedrag = ui::wSizeDrag::create(ui_ins, w_list);
	w_sizedrag->min_size() = Vec2(100.f);

	w_list->add_child(w_sizedrag, 1);

	for (auto i = 0; i < 20; i++)
	{
		auto item = ui::wListItem::create(ui_ins);
		item->w_btn()->text_col() = Bvec4(255);
		item->w_btn()->set_classic((L"item " + to_stdwstring(i)).c_str());
		w_list->add_child(item);
	}

	ui_ins->root()->add_child(w_list, 1);

	auto w_treenode1 = ui::wTreeNode::create(ui_ins, L"A", Bvec4(255, 255, 255, 255), Bvec4(200, 200, 200, 255));
	w_treenode1->pos$ = Vec2(300.f, 400.f);
	w_treenode1->w_btn()->text_col() = Bvec4(255);

	auto w_treenode2 = ui::wTreeNode::create(ui_ins, L"B", Bvec4(255, 255, 255, 255), Bvec4(200, 200, 200, 255));
	w_treenode2->w_btn()->text_col() = Bvec4(255);
	auto w_treenode3 = ui::wTreeNode::create(ui_ins, L"C", Bvec4(255, 255, 255, 255), Bvec4(200, 200, 200, 255));
	w_treenode3->w_btn()->text_col() = Bvec4(255);
	auto w_treenode4 = ui::wTreeNode::create(ui_ins, L"D", Bvec4(255, 255, 255, 255), Bvec4(200, 200, 200, 255));
	w_treenode4->w_btn()->text_col() = Bvec4(255);

	w_treenode1->w_items()->add_child(w_treenode2);
	w_treenode1->w_items()->add_child(w_treenode3);
	w_treenode3->w_items()->add_child(w_treenode4);

	ui_ins->root()->add_child(w_treenode1, 1);

	t_fps = ui::wText::create(ui_ins);
	t_fps->align$ = ui::AlignRightBottomNoPadding;
	t_fps->text_col() = Bvec4(0, 0, 0, 255);
	ui_ins->root()->add_child(t_fps, 1);

	t_fps->text_col() = Bvec4(255);

	//{
	//	auto new_widget = ui::Widget::create(ui_ins);

	//	auto xml = XmlFile::create("ui");

	//	auto n = xml->root_node->new_node("widget");
	//	auto u = typeinfo::cpp::find_udt(cH("ui::Widget"));

	//	serialize(n, u, t_fps, 1, new_widget);

	//	xml->save(L"d:/ui.xml");
	//	XmlFile::destroy(xml);

	//	ui::Widget::destroy(new_widget);
	//}

	//{
	//	auto xml = XmlFile::create_from_file(L"d:/ui.xml");

	//	auto u = typeinfo::cpp::find_udt(cH("ui::Widget"));

	//	unserialize(xml->root_node->node(0), u, t_fps);

	//	XmlFile::destroy(xml);
	//}

	app->run(do_run, {});

	return 0;
}
