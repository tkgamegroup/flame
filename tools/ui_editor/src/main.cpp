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

#include <flame/serialize.h>
#include <flame/ui/icon.h>
#include <flame/ui/style.h>

#include <flame/basic_app.h>

using namespace flame;
using namespace ui;

BasicApp app;

Instance *ui_ins_sandbox;

struct wHierachy : wDialog
{
	void init()
	{
		wDialog::init(true);

		add_data_storages({ });

		pos$ = Vec2(10.f);
		set_size(Vec2(100.f, 200.f));
		layout_type$ = LayoutVertical;

		//w_bottom() = wLayout::create(app.ui_ins);
		//w_bottom()->align$ = AlignBottomNoPadding;
		//w_bottom()->layout_type$ = LayoutHorizontal;
		//w_bottom()->item_padding$ = 4.f;
		//add_child(w_bottom(), 1);

		//w_widgets_list() = wCombo::create(app.ui_ins);
		//w_widgets_list()->align$ = AlignLittleEnd;

		//auto i_widget = wMenuItem::create(app.ui_ins, L"Widget");
		//w_widgets_list()->w_items()->add_child(i_widget);

		//w_bottom()->add_child(w_widgets_list());

		//w_add() = wButton::create(app.ui_ins);
		//w_add()->background_col$.w = 0;
		//w_add()->align$ = AlignLittleEnd;
		//w_add()->set_classic(Icon_PLUS_CIRCLE);
		//w_bottom()->add_child(w_add());

		//w_remove() = wButton::create(app.ui_ins);
		//w_remove()->background_col$.w = 0;
		//w_remove()->align$ = AlignLittleEnd;
		//w_remove()->set_classic(Icon_MINUS_CIRCLE);
		//w_bottom()->add_child(w_remove());

		//w_up() = wButton::create(app.ui_ins);
		//w_up()->background_col$.w = 0;
		//w_up()->align$ = AlignLittleEnd;
		//w_up()->set_classic(Icon_CHEVRON_CIRCLE_UP);
		//w_bottom()->add_child(w_up());

		//w_down() = wButton::create(app.ui_ins);
		//w_down()->background_col$.w = 0;
		//w_down()->align$ = AlignLittleEnd;
		//w_down()->set_classic(Icon_CHEVRON_CIRCLE_DOWN);
		//w_bottom()->add_child(w_down());
	}
};

wTree *w_hierachy_tree;
wTreeNode *w_hierachy_root;
wHierachy *w_hierachy;

struct wSandBox : wDialog
{
	void init()
	{
		wDialog::init(true);

		pos$ = Vec2(500.f, 10.f);
		set_size(Vec2(300.f, 300.f));
		inner_padding$ = Vec4(8.f);
		size_policy_hori$ = SizeFitLayout;
		size_policy_vert$ = SizeFitLayout;
		align$ = AlignLargeEnd;

		add_extra_draw([](const ParmPackage &_p) {
			auto &p = (Widget::ExtraDrawParm&)_p;

			ui_ins_sandbox->begin(1.f / 60.f);
			ui_ins_sandbox->end(p.canvas(), p.thiz()->pos$ * p.scl() + p.off() + Vec2(8.f));
		}, {});
	}
};

wSandBox *w_sandbox;

struct wInspector : wDialog
{
	void init()
	{
		wDialog::init(true);

		pos$ = Vec2(10.f, 500.f);
		set_size(Vec2(600.f, 400.f));
		layout_type$ = LayoutVertical;
	}
};

wInspector *w_inspector;

static UDT *widget_info;

void refresh_inspector()
{
	w_inspector->clear_children(0, 0, -1, true);

	auto sel = w_hierachy_tree->w_sel();
	if (sel && sel->name$ == "w")
	{
		auto p = sel->data_storages$[3].p();

		std::vector<Widget*> widgets_need_align;
		auto max_width = 0.f;
		const auto bullet_width = 16.f;

		for (auto i = 0; i < widget_info->item_count(); i++)
		{
			auto item = widget_info->item(i);

			auto w_item = Widget::createT<wLayout>(app.ui_ins, LayoutHorizontal, 8.f);
			w_item->inner_padding$.y = 20.f;
			w_item->size_policy_hori$ = SizeFitLayout;
			w_item->align$ = AlignLittleEnd;

			auto push_name_text = [&]() {
				auto t_name = Widget::createT<wText>(app.ui_ins);
				t_name->inner_padding$.x = bullet_width;
				t_name->align$ = AlignLittleEnd;
				t_name->text() = s2w(item->name());
				t_name->set_size_auto();
				max_width = max(t_name->size$.x, max_width);
				w_item->add_child(t_name, 0, -1, true);

				widgets_need_align.push_back(t_name);

				return t_name;
			};
			auto push_name_node = [&]() {
				auto n_name = Widget::createT<wTreeNode>(app.ui_ins, s2w(item->name()).c_str());
				n_name->align$ = AlignLittleEnd;
				max_width = max(n_name->size$.x, max_width);
				w_item->add_child(n_name, 0, -1, true);

				widgets_need_align.push_back(n_name);

				return n_name;
			};

			switch (item->tag())
			{
			case VariableTagEnumSingle: case VariableTagEnumMulti: case VariableTagVariable:
				push_name_text();
				Widget::create_from_typeinfo(app.ui_ins, item, p, w_item);
				break;
			case VariableTagArrayOfVariable:
			{
				auto n = push_name_node();
			}
				break;
			case VariableTagArrayOfPointer:
			{
				auto n = push_name_node();

				switch (item->type_hash())
				{
				case cH("Function"):
					break;
				}
			}
				break;
			}

			w_inspector->add_child(w_item, 0, -1, true);
		}

		for (auto w : widgets_need_align)
			w->set_width(max_width);
	}
}

wTreeNode *create_hierachy_treenode(const wchar_t *name, Widget *p)
{
	auto n = Widget::createT<wTreeNode>(app.ui_ins, name, w_hierachy_tree);
	n->add_data_storages({ p });
	n->name$ = "w";

	return n;
}

void refresh_hierachy_node(wTreeNode *dst, Widget *src)
{
	for (auto i = 0; i < 2; i++)
	{
		auto n_layer = Widget::createT<wTreeNode>(app.ui_ins, (L"layer " + to_stdwstring(i)).c_str(), w_hierachy_tree);
		dst->w_items()->add_child(n_layer, 0, -1, true);

		auto &children = i == 0 ? src->children_1$ : src->children_2$;
		for (auto j = 0; j < children.size; j++)
		{
			auto c = children[j];
			auto n = create_hierachy_treenode(s2w(c->name$.v).c_str(), c);
			n_layer->w_items()->add_child(n, 0, -1, true);
			refresh_hierachy_node(n, c);
		}
	}
}

void refresh_hierachy()
{
	w_hierachy_root->w_items()->clear_children(0, 0, -1, true);
	
	refresh_hierachy_node(w_hierachy_root, ui_ins_sandbox->root());
}

extern "C" __declspec(dllexport) int main()
{
	app.create("UI Editor", Ivec2(1280, 720), WindowFrame | WindowResizable);

	app.canvas->set_clear_color(Bvec4(200, 200, 200, 255));

	ui_ins_sandbox = Instance::create();
	ui_ins_sandbox->root()->background_col$ = Bvec4(0, 0, 0, 255);
	ui_ins_sandbox->on_resize(Ivec2(200));

	w_hierachy_tree = Widget::createT<wTree>(app.ui_ins);
	w_hierachy_tree->add_listener(Widget::ListenerChanged, [](const ParmPackage &_p) {
		refresh_inspector();
	}, w_hierachy_tree, {});
	w_hierachy_root = create_hierachy_treenode(L"root", ui_ins_sandbox->root());
	w_hierachy_tree->add_child(w_hierachy_root);

	w_hierachy = Widget::createT<wHierachy>(app.ui_ins);
	w_hierachy->add_child(w_hierachy_tree);
	app.ui_ins->root()->add_child(w_hierachy);

	w_sandbox = Widget::createT<wSandBox>(app.ui_ins);
	app.ui_ins->root()->add_child(w_sandbox);

	widget_info = find_udt(cH("ui::Widget"));

	w_inspector = Widget::createT<wInspector>(app.ui_ins);
	app.ui_ins->root()->add_child(w_inspector);

	auto t_text = Widget::createT<wText>(ui_ins_sandbox);
	t_text->align$ = AlignTopNoPadding;
	t_text->name$ = "Hello World";
	t_text->text() = L"Hello World";
	t_text->set_size_auto();
	ui_ins_sandbox->root()->add_child(t_text, 1);

	refresh_hierachy();

	app.run();

	return 0;
}
