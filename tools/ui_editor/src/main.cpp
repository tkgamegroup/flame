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
#include <flame/graphics/renderpass.h>
#include <flame/ui/icon.h>
#include <flame/ui/widget_private.h>
#include <flame/basic_app.h>

using namespace flame;

BasicApp app;

struct wBasicWindow : ui::wLayout
{
	void init()
	{
		resize_data_storage(1);

		background_col$ = Colorf(0.5f, 0.5f, 0.5f, 0.9f);
		event_attitude$ = ui::EventAccept;
		size_policy_hori$ = ui::SizeFitLayout;
		size_policy_vert$ = ui::SizeFitLayout;
		layout_type$ = ui::LayoutVertical;

		w_sizedrag() = ui::wSizeDrag::create(instance(), this);
		w_sizedrag()->min_size() = Vec2(10.f);
		set_size(Vec2(300.f, 400.f));
		add_child(w_sizedrag(), 1);

		add_listener(cH("mouse move"), [](CommonData *d) {
			auto disp = Vec2::from(d[0].f);
			auto thiz = (wBasicWindow*)d[1].p;

			if (thiz == thiz->instance()->dragging_widget())
				thiz->pos$ += disp / thiz->parent()->scale$;
		}, "this:basicwindow", this);
	}

	ui::wSizeDragPtr &w_sizedrag()
	{
		return *((ui::wSizeDragPtr*)&data_storage(0).p);
	}

	static wBasicWindow *create(ui::Instance *ui)
	{
		auto w = (wBasicWindow*)ui::wLayout::create(ui);
		w->init();
		return w;
	}
};

ui::Instance *ui_sandbox;
ui::wTreeNode *w_sel = nullptr;

void refresh_inspector();

struct wHierachy : wBasicWindow
{
	void init()
	{
		resize_data_storage(7);

		add_listener(cH("double clicked"), [](CommonData *d) {
			w_sel = nullptr;
			refresh_inspector();
		}, "");

		w_bottom() = ui::wLayout::create(app.ui);
		w_bottom()->align$ = ui::AlignBottomNoPadding;
		w_bottom()->layout_type$ = ui::LayoutHorizontal;
		w_bottom()->item_padding$ = 4.f;
		add_child(w_bottom(), 1);

		w_widgets_list() = ui::wCombo::create(app.ui);
		w_widgets_list()->align$ = ui::AlignLittleEnd;

		auto i_widget = ui::wMenuItem::create(app.ui, L"Widget");
		w_widgets_list()->w_items()->add_child(i_widget);

		w_bottom()->add_child(w_widgets_list());

		w_add() = ui::wButton::create(app.ui);
		w_add()->align$ = ui::AlignLittleEnd;
		w_add()->set_classic(ui::Icon_PLUS_CIRCLE);
		w_bottom()->add_child(w_add());

		w_remove() = ui::wButton::create(app.ui);
		w_remove()->align$ = ui::AlignLittleEnd;
		w_remove()->set_classic(ui::Icon_MINUS_CIRCLE);
		w_bottom()->add_child(w_remove());

		w_up() = ui::wButton::create(app.ui);
		w_up()->align$ = ui::AlignLittleEnd;
		w_up()->set_classic(ui::Icon_CHEVRON_CIRCLE_UP);
		w_bottom()->add_child(w_up());

		w_down() = ui::wButton::create(app.ui);
		w_down()->align$ = ui::AlignLittleEnd;
		w_down()->set_classic(ui::Icon_CHEVRON_CIRCLE_DOWN);
		w_bottom()->add_child(w_down());
	}

	ui::wLayoutPtr &w_bottom()
	{
		return *((ui::wLayoutPtr*)&data_storage(1).p);
	}

	ui::wComboPtr &w_widgets_list()
	{
		return *((ui::wComboPtr*)&data_storage(2).p);
	}

	ui::wButtonPtr &w_add()
	{
		return *((ui::wButtonPtr*)&data_storage(3).p);
	}

	ui::wButtonPtr &w_remove()
	{
		return *((ui::wButtonPtr*)&data_storage(4).p);
	}

	ui::wButtonPtr &w_up()
	{
		return *((ui::wButtonPtr*)&data_storage(5).p);
	}

	ui::wButtonPtr &w_down()
	{
		return *((ui::wButtonPtr*)&data_storage(6).p);
	}

	static wHierachy *create(ui::Instance *ui)
	{
		auto w = (wHierachy*)wBasicWindow::create(ui);
		w->init();
		return w;
	}
};

ui::wTreeNode *w_root;
wHierachy *w_hierachy;

struct wInstanceWrapper : ui::Widget
{
	void init()
	{
		size_policy_hori$ = ui::SizeFitLayout;
		size_policy_vert$ = ui::SizeFitLayout;
		align$ = ui::AlignLargeEnd;

		add_draw_command([](CommonData *d) {
			auto c = (ui::Canvas*)d[0].p;
			auto off = Vec2::from(d[1].f);
			auto scl = d[2].f[0];
			auto thiz = (wInstanceWrapper*)d[3].p;

			auto s = Ivec2(thiz->size$);
			if (s != ui_sandbox->size())
				ui_sandbox->on_resize(Ivec2(s));

			ui_sandbox->begin(1.f / 60.f);
			ui_sandbox->end(c, thiz->pos$ * scl + off);
		}, "this:instancewrapper", this);
	}

	static wInstanceWrapper *create(ui::Instance *ui)
	{
		auto w = (wInstanceWrapper*)ui::Widget::create(ui);
		w->init();
		return w;
	}
};

wInstanceWrapper *w_wrapper;

struct wSandBox : wBasicWindow
{
	void init()
	{
		pos$.x = 500.f;
		inner_padding$ = Vec4(8.f);
	}

	ui::wSizeDragPtr &w_sizedrag()
	{
		return *((ui::wSizeDragPtr*)&data_storage(0).p);
	}

	static wSandBox *create(ui::Instance *ui)
	{
		auto w = (wSandBox*)wBasicWindow::create(ui);
		w->init();
		return w;
	}
};

wSandBox *w_sandbox;

struct wInspector : wBasicWindow
{
	void init()
	{
		pos$.y = 500.f;
	}

	static wInspector *create(ui::Instance *ui)
	{
		auto w = (wInspector*)wBasicWindow::create(ui);
		w->init();
		return w;
	}
};

wInspector *w_inspector;

static typeinfo::cpp::UDT *widget_info;

void refresh_inspector()
{
	w_inspector->clear_children(0, 0, -1, true);

	if (w_sel && w_sel->name_hash == cH("widget node"))
	{
		auto p = (ui::WidgetPrivate*)w_sel->data_storage(3).p;

		for (auto i = 0; i < widget_info->item_count(); i++)
		{
			auto t = ui::wText::create(app.ui);
			t->align$ = ui::AlignLittleEnd;
			t->text_col() = Bvec4(0, 0, 0, 255);

			auto v = widget_info->item(i);
			std::wstring text = s2w(v->name());
			text += L": ";
			switch (v->tag())
			{
			case typeinfo::cpp::VariableTagPointer:
				switch (v->type_hash())
				{
				case cH("char"):
					text += s2w(*(char**)((char*)p + v->offset()));
					break;
				}
				break;
			}

			t->set_text_and_size(text.c_str());
			w_inspector->add_child(t, 0, -1, true);
		}

		//auto n_drawlist = ui::wTreeNode::create(app.ui, L"draw list");
		//for (auto c : p->draw_commands)
		//{
		//	const char *filename;
		//	int line_beg;
		//	int line_end;
		//	auto t_cmd = ui::wText::create(app.ui);
		//	t_cmd->align = ui::AlignLittleEnd;
		//	std::wstring text;
		//	auto id = get_PF_props(c->pf, &filename, &line_beg, &line_end);
		//	if (id != 0)
		//		text = s2w(filename) + L": " + to_wstring(line_beg) + L" - " + to_wstring(line_end);
		//	else
		//		text = L"unregistered function";
		//	t_cmd->set_text_and_size(text.c_str());
		//	n_drawlist->w_items()->add_child(t_cmd, 0, -1, true);
		//}
		//w_inspector->add_child(n_drawlist, 0, -1, true);
	}
}

void set_widget_tree_node(ui::wTreeNode *n, ui::Widget *p)
{
	n->resize_data_storage(4);

	n->set_name("widget node");

	n->data_storage(3).p = p;

	n->w_btn()->add_listener(cH("clicked"), [](CommonData *d) {
		auto n = (ui::wTreeNode*)d[0].p;

		w_sel = n;
		refresh_inspector();
	}, "p:treenode", n);
	
	n->w_btn()->add_style(0, [](CommonData *d) {
		auto w = (ui::wText*)d[0].p;

		if (w_sel && w_sel->w_btn() == w)
			w->background_col$ = Bvec4(40, 100, 180, 255);
		else
			w->background_col$ = Bvec4(0);
	}, "");
}

void refresh_hierachy_node(ui::wTreeNode *dst, ui::Widget *src)
{
	for (auto i = 0; i < 2; i++)
	{
		auto label = ui::wText::create(app.ui);
		label->align$ = ui::AlignLittleEnd;
		label->set_text_and_size((L"layer " + to_wstring(i)).c_str());
		dst->w_items()->add_child(label, 0, -1, true);

		for (auto j = 0; j < src->children_count(i); j++)
		{
			auto c = src->child(i, j);
			auto n = ui::wTreeNode::create(app.ui, s2w(c->name$).c_str());
			set_widget_tree_node(n, c);
			dst->w_items()->add_child(n, 0, -1, true);
			refresh_hierachy_node(n, c);
		}
	}
}

void refresh_hierachy()
{
	w_root->w_items()->clear_children(0, 0, -1, true);
	
	refresh_hierachy_node(w_root, ui_sandbox->root());
}

extern "C" __declspec(dllexport) int main()
{
	app.create("Ui Editor", Ivec2(1280, 720), WindowFrame | WindowResizable);

	app.canvas->clear_values->set(0, Bvec4(200, 200, 200, 0));

	ui_sandbox = ui::Instance::create();
	ui_sandbox->root()->background_col$ = Bvec4(200, 200, 200, 255);

	w_root = ui::wTreeNode::create(app.ui, L"root");
	set_widget_tree_node(w_root, ui_sandbox->root());

	w_hierachy = wHierachy::create(app.ui);
	w_hierachy->add_child(w_root);
	app.ui->root()->add_child(w_hierachy);

	w_wrapper = wInstanceWrapper::create(app.ui);
	w_sandbox = wSandBox::create(app.ui);
	w_sandbox->add_child(w_wrapper);
	app.ui->root()->add_child(w_sandbox);

	widget_info = typeinfo::find_cpp_serializable(cH("ui::Widget"));

	w_inspector = wInspector::create(app.ui);
	app.ui->root()->add_child(w_inspector);

	auto t_text = ui::wText::create(ui_sandbox);
	t_text->set_name("Hello World");
	t_text->align$ = ui::AlignTopNoPadding;
	t_text->set_text_and_size(L"Hello World");
	ui_sandbox->root()->add_child(t_text, 1);

	refresh_hierachy();

	app.run();

	return 0;
}
