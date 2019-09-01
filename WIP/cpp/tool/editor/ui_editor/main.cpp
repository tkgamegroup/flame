#include <flame/serialize.h>
#include <flame/ui/icon.h>

BasicApp app;
UDT *widget_info;
Instance *ui_ins_sandbox;

FLAME_WIDGET_BEGIN(wInspector, wDialog)
	void init();
	void refresh();
FLAME_WIDGET_END
wInspector *w_inspector;

FLAME_WIDGET_BEGIN(wContextMenu, wMenu)
	void init();
FLAME_WIDGET_END
wContextMenu *context_menu;

FLAME_WIDGET_BEGIN(wHierachy, wDialog)
	FLAME_WIDGET_DATA(wTreePtr, w_tree, p)
	FLAME_WIDGET_DATA(wTreeNodePtr, w_root, p)
	FLAME_WIDGET_SEPARATOR
	void init();
	wTreeNode *create_node(const wchar_t *name, Widget *p);
	void refresh_node(wTreeNode *dst, Widget *src);
	void refresh();
FLAME_WIDGET_END
wHierachy *w_hierachy;

FLAME_WIDGET_BEGIN(wSandBox, wDialog)
	void init();
FLAME_WIDGET_END
wSandBox *w_sandbox;

void wInspector::init()
{
	wDialog::init(true);

	pos$ = Vec2(10.f, 500.f);
	set_size(Vec2(600.f, 400.f));
	layout_type$ = LayoutVertical;
}

void wInspector::refresh()
{
	clear_children(0, 0, -1, true);

	auto sel = w_hierachy->w_tree()->w_sel();
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

			add_child(w_item, 0, -1, true);
		}

		for (auto w : widgets_need_align)
			w->set_width(max_width);
	}
}

void wContextMenu::init()
{
	wMenu::init(L"", true);

	auto w_menu_add = Widget::createT<wMenu>(app.ui_ins, L"Add");

	auto w_add_widget = Widget::createT<wMenuItem>(app.ui_ins, L"Widget");
	w_menu_add->w_items()->add_child(w_add_widget);

	auto w_add_layout = Widget::createT<wMenuItem>(app.ui_ins, L"Layout");
	w_menu_add->w_items()->add_child(w_add_layout);

	w_items()->add_child(w_menu_add);

	auto w_remove = Widget::createT<wMenuItem>(app.ui_ins, L"Remove");
	w_items()->add_child(w_remove);

	auto w_up = Widget::createT<wMenuItem>(app.ui_ins, L"Up");
	w_items()->add_child(w_up);

	auto w_down = Widget::createT<wMenuItem>(app.ui_ins, L"Down");
	w_items()->add_child(w_down);

	auto w_unparent = Widget::createT<wMenuItem>(app.ui_ins, L"Unparent");
	w_items()->add_child(w_unparent);
}

void wHierachy::init()
{
	wDialog::init(true);

	add_data_storages({ nullptr, nullptr, nullptr });

	pos$ = Vec2(10.f);
	set_size(Vec2(100.f, 200.f));
	layout_type$ = LayoutVertical;

	w_tree() = Widget::createT<wTree>(app.ui_ins);
	w_tree()->add_listener(Widget::ListenerChanged, [](const ParmPackage &_p) {
		w_inspector->refresh();
	}, w_tree(), {});

	w_root() = create_node(L"root", ui_ins_sandbox->root());

	w_tree()->add_child(w_root());

	add_child(w_tree());
}

wTreeNode *wHierachy::create_node(const wchar_t *name, Widget *p)
{
	auto n = Widget::createT<wTreeNode>(app.ui_ins, name, w_tree());
	if (p)
	{
		n->add_data_storages({ p });
		n->name$ = "w";
	}
	n->w_title()->add_listener(Widget::ListenerMouse, [](const ParmPackage &_p) {
		auto &p = (Widget::MouseListenerParm&)_p;
		if (!(p.action() == KeyStateDown && p.key() == Mouse_Right))
			return;

		context_menu->popup(p.value());
	}, n, {});

	return n;
}

void wHierachy::refresh_node(wTreeNode *dst, Widget *src)
{
	for (auto i = 0; i < 2; i++)
	{
		auto n_layer = create_node((L"layer " + to_stdwstring(i)).c_str(), nullptr);
		dst->w_items()->add_child(n_layer, 0, -1, true);

		auto &children = i == 0 ? src->children_1$ : src->children_2$;
		for (auto j = 0; j < children.size; j++)
		{
			auto c = children[j];
			auto n = create_node(s2w(c->name$.v).c_str(), c);
			n_layer->w_items()->add_child(n, 0, -1, true);
			refresh_node(n, c);
		}
	}
}

void wHierachy::refresh()
{
	w_root()->w_items()->clear_children(0, 0, -1, true);
	refresh_node(w_root(), ui_ins_sandbox->root());
}

void wSandBox::init()
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

extern "C" __declspec(dllexport) int main()
{
	ui_ins_sandbox = Instance::create();
	ui_ins_sandbox->root()->background_col$ = Bvec4(0, 0, 0, 255);
	ui_ins_sandbox->on_resize(Ivec2(200));

	context_menu = Widget::createT< wContextMenu>(app.ui_ins);

	w_hierachy = Widget::createT<wHierachy>(app.ui_ins);
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

	w_hierachy->refresh();

	app.run();

	return 0;
}
