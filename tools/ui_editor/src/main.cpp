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

BasicApp app;

ui::Instance *ui_ins_sandbox;
ui::wTreeNode *w_sel = nullptr;

void refresh_inspector();

struct wHierachy : ui::wDialog
{
	void init()
	{
		pos$ = Vec2(10.f);
		set_size(Vec2(100.f, 200.f));
		layout_type$ = ui::LayoutVertical;

		add_data_storages("p p p p p p");

		add_listener(cH("double clicked"), [](CommonData *d) {
			w_sel = nullptr;
			refresh_inspector();
		}, {});

		w_bottom() = ui::wLayout::create(app.ui_ins);
		w_bottom()->align$ = ui::AlignBottomNoPadding;
		w_bottom()->layout_type$ = ui::LayoutHorizontal;
		w_bottom()->item_padding$ = 4.f;
		add_child(w_bottom(), 1);

		w_widgets_list() = ui::wCombo::create(app.ui_ins);
		w_widgets_list()->align$ = ui::AlignLittleEnd;

		auto i_widget = ui::wMenuItem::create(app.ui_ins, L"Widget");
		w_widgets_list()->w_items()->add_child(i_widget);

		w_bottom()->add_child(w_widgets_list());

		w_add() = ui::wButton::create(app.ui_ins);
		w_add()->background_col$.w = 0;
		w_add()->align$ = ui::AlignLittleEnd;
		w_add()->set_classic(ui::Icon_PLUS_CIRCLE);
		w_bottom()->add_child(w_add());

		w_remove() = ui::wButton::create(app.ui_ins);
		w_remove()->background_col$.w = 0;
		w_remove()->align$ = ui::AlignLittleEnd;
		w_remove()->set_classic(ui::Icon_MINUS_CIRCLE);
		w_bottom()->add_child(w_remove());

		w_up() = ui::wButton::create(app.ui_ins);
		w_up()->background_col$.w = 0;
		w_up()->align$ = ui::AlignLittleEnd;
		w_up()->set_classic(ui::Icon_CHEVRON_CIRCLE_UP);
		w_bottom()->add_child(w_up());

		w_down() = ui::wButton::create(app.ui_ins);
		w_down()->background_col$.w = 0;
		w_down()->align$ = ui::AlignLittleEnd;
		w_down()->set_classic(ui::Icon_CHEVRON_CIRCLE_DOWN);
		w_bottom()->add_child(w_down());
	}

	ui::wLayoutPtr &w_bottom()
	{
		return *((ui::wLayoutPtr*)&data_storages$[1].p());
	}

	ui::wComboPtr &w_widgets_list()
	{
		return *((ui::wComboPtr*)&data_storages$[2].p());
	}

	ui::wButtonPtr &w_add()
	{
		return *((ui::wButtonPtr*)&data_storages$[3].p());
	}

	ui::wButtonPtr &w_remove()
	{
		return *((ui::wButtonPtr*)&data_storages$[4].p());
	}

	ui::wButtonPtr &w_up()
	{
		return *((ui::wButtonPtr*)&data_storages$[5].p());
	}

	ui::wButtonPtr &w_down()
	{
		return *((ui::wButtonPtr*)&data_storages$[6].p());
	}

	static wHierachy *create(ui::Instance *ui)
	{
		auto w = (wHierachy*)ui::wDialog::create(ui, true);
		w->init();
		return w;
	}
};

ui::wTreeNode *w_root;
wHierachy *w_hierachy;

struct wSandBox : ui::wDialog
{
	void init()
	{
		pos$ = Vec2(500.f, 10.f);
		set_size(Vec2(300.f, 300.f));
		inner_padding$ = Vec4(8.f);
		size_policy_hori$ = ui::SizeFitLayout;
		size_policy_vert$ = ui::SizeFitLayout;
		align$ = ui::AlignLargeEnd;

		add_extra_draw_command([](CommonData *d) {
			auto &c = *(ui::Canvas**)&d[0].p();
			auto &off = d[1].f2();
			auto &scl = d[2].f1();
			auto &thiz = *(wSandBox**)&d[3].p();

			ui_ins_sandbox->begin(1.f / 60.f);
			ui_ins_sandbox->end(c, thiz->pos$ * scl + off + Vec2(8.f));
		}, { this });
	}

	static wSandBox *create(ui::Instance *ui)
	{
		auto w = (wSandBox*)ui::wDialog::create(ui, true);
		w->init();
		return w;
	}
};

wSandBox *w_sandbox;

struct wInspector : ui::wDialog
{
	void init()
	{
		pos$ = Vec2(10.f, 500.f);
		set_size(Vec2(600.f, 400.f));
		layout_type$ = ui::LayoutVertical;
	}

	static wInspector *create(ui::Instance *ui)
	{
		auto w = (wInspector*)ui::wDialog::create(ui, true);
		w->init();
		return w;
	}
};

wInspector *w_inspector;

static UDT *widget_info;

ui::wTreeNode *create_treenode(const wchar_t *name)
{
	auto n = ui::wTreeNode::create(app.ui_ins, name, Bvec4(0, 0, 0, 255), Bvec4(255, 255, 255, 255));

	n->w_btn()->add_style([](CommonData *d) {
		auto &w = *(ui::wText**)&d[0].p();

		if (w_sel && w_sel->w_btn() == w)
			w->background_col$ = Bvec4(40, 100, 180, 255);
		else
			w->background_col$ = Bvec4(0);
	}, {});

	return n;
}

void refresh_inspector()
{
	w_inspector->clear_children(0, 0, -1, true);

	if (w_sel && w_sel->name$ == "w")
	{
		auto p = w_sel->data_storages$[3].p();

		std::vector<ui::Widget*> widgets_need_align;
		auto max_width = 0.f;
		const auto bullet_width = 16.f;

		for (auto i = 0; i < widget_info->item_count(); i++)
		{
			auto item = widget_info->item(i);

			auto w_item = ui::wLayout::create(app.ui_ins);
			w_item->inner_padding$.y = 20.f;
			w_item->size_policy_hori$ = ui::SizeFitLayout;
			w_item->align$ = ui::AlignLittleEnd;
			w_item->layout_type$ = ui::LayoutHorizontal;
			w_item->item_padding$ = 8.f;

			auto push_name_text = [&]() {
				auto t_name = ui::wText::create(app.ui_ins);
				t_name->inner_padding$.x = bullet_width;
				t_name->align$ = ui::AlignLittleEnd;
				t_name->text() = s2w(item->name());
				t_name->set_size_auto();
				max_width = max(t_name->size$.x, max_width);
				w_item->add_child(t_name, 0, -1, true);

				widgets_need_align.push_back(t_name);

				return t_name;
			};
			auto push_name_node = [&]() {
				auto n_name = create_treenode(s2w(item->name()).c_str());
				n_name->align$ = ui::AlignLittleEnd;
				max_width = max(n_name->size$.x, max_width);
				w_item->add_child(n_name, 0, -1, true);

				widgets_need_align.push_back(n_name);

				return n_name;
			};

			switch (item->tag())
			{
			case VariableTagEnumSingle: case VariableTagEnumMulti: case VariableTagVariable:
				push_name_text();
				ui::Widget::create_from_typeinfo(app.ui_ins, item, p, w_item);
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

ui::wTreeNode *create_hierachy_treenode(const wchar_t *name)
{
	auto n = create_treenode(name);

	n->w_btn()->add_listener(ui::Widget::ListenerMouse, [](CommonData *d) {
		auto &n = *(ui::wTreeNode**)&d[0].p();

		w_sel = n;
		refresh_inspector();
	}, n, {});

	return n;
}

void set_widget_tree_node(ui::wTreeNode *n, ui::Widget *p)
{
	n->name$ = "w";
	n->add_data_storages("p");
	n->data_storages$[3].p() = p;
}

void refresh_hierachy_node(ui::wTreeNode *dst, ui::Widget *src)
{
	for (auto i = 0; i < 2; i++)
	{
		auto n_layer = create_hierachy_treenode((L"layer " + to_stdwstring(i)).c_str());
		dst->w_items()->add_child(n_layer, 0, -1, true);

		auto &children = i == 0 ? src->children_1$ : src->children_2$;
		for (auto j = 0; j < children.size; j++)
		{
			auto c = children[j];
			auto n = create_hierachy_treenode(s2w(c->name$.v).c_str());
			set_widget_tree_node(n, c);
			n_layer->w_items()->add_child(n, 0, -1, true);
			refresh_hierachy_node(n, c);
		}
	}
}

void refresh_hierachy()
{
	w_root->w_items()->clear_children(0, 0, -1, true);
	
	refresh_hierachy_node(w_root, ui_ins_sandbox->root());
}

extern "C" __declspec(dllexport) int main()
{
	app.create("UI Editor", Ivec2(1280, 720), WindowFrame | WindowResizable);

	app.canvas->set_clear_color(Bvec4(200, 200, 200, 0));

	ui_ins_sandbox = ui::Instance::create();
	ui_ins_sandbox->root()->background_col$ = Bvec4(0, 0, 0, 255);
	ui_ins_sandbox->on_resize(Ivec2(200));

	w_root = create_hierachy_treenode(L"root");
	set_widget_tree_node(w_root, ui_ins_sandbox->root());

	w_hierachy = wHierachy::create(app.ui_ins);
	w_hierachy->add_child(w_root);
	app.ui_ins->root()->add_child(w_hierachy);

	w_sandbox = wSandBox::create(app.ui_ins);
	app.ui_ins->root()->add_child(w_sandbox);

	widget_info = find_udt(cH("ui::Widget"));

	w_inspector = wInspector::create(app.ui_ins);
	app.ui_ins->root()->add_child(w_inspector);

	auto t_text = ui::wText::create(ui_ins_sandbox);
	t_text->align$ = ui::AlignTopNoPadding;
	t_text->name$ = "Hello World";
	t_text->text() = L"Hello World";
	t_text->set_size_auto();
	ui_ins_sandbox->root()->add_child(t_text, 1);

	refresh_hierachy();

	app.run();

	return 0;
}
