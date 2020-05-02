#include <flame/universe/utils/typeinfo.h>

#include "scene_editor.h"

struct SetterCapture
{
	void* p;
	void* o;
	void* f;
};

template <class T>
void create_edit(SetterCapture* c)
{
	auto& ui = scene_editor.window->ui;

	ui.e_drag_edit();

	ui.parents.top()->add_component(new_object<cDigitalDataTracker<T>>(c->p, [](Capture& c, T v, bool exit_editing) {
		auto& capture = c.data<SetterCapture>();
		if (capture.f)
			Setter_t<T>::set_s(capture.o, capture.f, v, scene_editor.inspector);
		else
			*(T*)capture.p = v;
	}, Capture().set_data(c)));
}

template <uint N, class T>
void create_vec_edit(SetterCapture* c)
{
	auto& ui = scene_editor.window->ui;

	for (auto i = 0; i < N; i++)
		ui.e_drag_edit();

	auto p = ui.parents.top();
	p->get_component(cLayout)->type = LayoutHorizontal;
	p->add_component(new_object<cDigitalVecDataTracker<N, T>>(c->p, [](Capture& c, const Vec<N, T>& v, bool exit_editing) {
		auto& capture = c.data<SetterCapture>();
		if (capture.f)
			Setter_t<Vec<N, T>>::set_s(capture.o, capture.f, (Vec<N, T>*)&v, scene_editor.inspector);
		else
			*(Vec<N, T>*)capture.p = v;
	}, Capture().set_data(c)));
}

cInspector::cInspector() :
	Component("cInspector")
{
	auto& ui = scene_editor.window->ui;

	ui.next_element_padding = 4.f;
	auto e_page = ui.e_begin_docker_page(L"Inspector").second;
	{
		auto c_layout = ui.c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

	ui.e_begin_scrollbar(ScrollbarVertical, true);
		e_layout = ui.e_empty();
		{
			ui.c_element()->clip_flags = ClipChildren;
			auto cl = ui.c_layout(LayoutVertical);
			cl->item_padding = 4.f;
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			ui.c_aligner(AlignMinMax, AlignMinMax);
		}
	ui.e_end_scrollbar();

	refresh();
}

cInspector::~cInspector()
{
	scene_editor.inspector = nullptr;
}

struct cComponentTracker : Component
{
	std::unordered_map<uint, cDataTracker*> vs;

	Component* t;
	void* l;

	cComponentTracker(Component* t) :
		t(t),
		Component("cComponentTracker")
	{
		l = t->data_changed_listeners.add([](Capture& c, uint hash, void* sender) {
			if (sender == scene_editor.inspector)
				return true;
			auto thiz = c.thiz<cComponentTracker>();
			auto it = thiz->vs.find(hash);
			if (it != thiz->vs.end())
				it->second->update_view();
			return true;
		}, Capture().set_thiz(this));
	}

	~cComponentTracker()
	{
		if (!entity->dying_)
			t->data_changed_listeners.remove(l);
	}
};

void cInspector::refresh()
{
	auto& ui = scene_editor.window->ui;

	e_layout->remove_children(0, -1);

	ui.parents.push(e_layout);
	if (!scene_editor.selected)
		ui.e_text(L"Nothing Selected");
	else
	{
		ui.e_begin_layout(LayoutHorizontal, 4.f);
		ui.e_text(L"name");
		ui.e_edit(100.f, s2w(scene_editor.selected->name()).c_str(), true, true)->get_component(cText)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
			if (hash == FLAME_CHASH("text"))
			{
				auto text = c.current<cText>()->text.v;
				scene_editor.selected->set_name(w2s(text).c_str());
				if (scene_editor.hierarchy)
				{
					auto item = scene_editor.hierarchy->find_item(scene_editor.selected);
					if (item->get_component(cTreeNode))
						item->child(0)->get_component(cText)->set_text(text);
					else
						item->get_component(cText)->set_text(text);
				}
			}
			return true;
		}, Capture());
		ui.e_end_layout();

		ui.e_begin_layout(LayoutHorizontal, 4.f);
		ui.e_text(L"visible");
		ui.e_checkbox(scene_editor.selected->visible_)->get_component(cCheckbox)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
			if (hash == FLAME_CHASH("checked"))
				scene_editor.selected->set_visible(c.current<cCheckbox>()->checked);
			return true;
		}, Capture());
		ui.e_end_layout();

		auto components = scene_editor.selected->get_components();
		for (auto i = 0; i < components.s; i++)
		{
			auto component = components.v[i];

			auto udt = find_udt(FLAME_HASH((std::string("flame::") + component->name).c_str()));
			auto module = udt->db()->module();

			ui.next_element_padding = 4.f;
			auto e_component = ui.e_begin_layout(LayoutVertical, 2.f);
			ui.c_aligner(AlignMinMax, 0);

			auto c_component_tracker = new cComponentTracker(component);
			c_component_tracker->id = FLAME_HASH(component->name);
			e_component->add_component(c_component_tracker);

			ui.e_begin_layout(LayoutHorizontal, 4.f);
			ui.e_text(s2w(component->name).c_str())->get_component(cText)->color = Vec4c(30, 40, 160, 255);
				struct Capturing
				{
					Entity* e;
					Component* c;
				}capture;
				capture.e = e_component;
				capture.c = component;
				ui.push_style(ButtonColorNormal, common(Vec4c(0)));
				ui.push_style(ButtonColorHovering, common(ui.style(FrameColorHovering).c));
				ui.push_style(ButtonColorActive, common(ui.style(FrameColorActive).c));
				ui.e_button(L"X", [](Capture& c) {
					looper().add_event([](Capture& c) {
						auto& capture = c.data<Capturing>();
						capture.e->parent()->remove_child(capture.e);
						capture.c->entity->remove_component(capture.c);
					}, Capture().set_data(&c.data<Capturing>()));
				}, Capture().set_data(&capture));
				ui.pop_style(ButtonColorNormal);
				ui.pop_style(ButtonColorHovering);
				ui.pop_style(ButtonColorActive);
			ui.e_end_layout();

			for (auto i = 0; i < udt->variable_count(); i++)
			{
				auto v = udt->variable(i);
				auto type = v->type();
				auto base_hash = type->base_hash();
				auto pdata = (char*)component + v->offset();

				ui.e_begin_layout(LayoutHorizontal, 4.f);
				ui.e_text(s2w(v->name()).c_str());

				auto e_data = ui.e_begin_layout(LayoutVertical, 4.f);

				auto f_set = udt->find_function((std::string("set_") + v->name()).c_str());
				auto f_set_addr = f_set ? (char*)module + (uint)f_set->rva() : nullptr;

				//switch (type->tag())
				//{
				//case TypeEnumSingle:
				//{
				//	auto info = find_enum(base_hash);
				//	ui.e_begin_combobox();
				//	for (auto i = 0; i < info->item_count(); i++)
				//		ui.e_combobox_item(s2w(info->item(i)->name()).c_str());
				//	ui.e_end_combobox();

				//	e_data->add_component(new_object<cEnumSingleDataTracker>(pdata, info, [](Capture& c, int v) {
				//		;
				//	}, Capture()));
				//}
				//break;
				//case TypeEnumMulti:
				//{
				//	auto info = find_enum(base_hash);
				//	for (auto i = 0; i < info->item_count(); i++)
				//	{
				//		ui.e_begin_layout(LayoutHorizontal, 4.f);
				//		auto str = s2w(info->item(i)->name());
				//		ui.e_checkbox();
				//		ui.e_text(str.c_str());
				//		ui.e_end_layout();
				//	}

				//	e_data->add_component(new_object<cEnumMultiDataTracker>(pdata, info, [](Capture& c, int v) {
				//		;
				//	}, Capture()));
				//}
				//break;
				//case TypeData:
				//{
				//	SetterCapture setter_capture;
				//	setter_capture.p = pdata;
				//	setter_capture.o = component;
				//	setter_capture.f = f_set_addr;
				//	switch (base_hash)
				//	{
				//	case FLAME_CHASH("bool"):
				//		ui.e_checkbox();

				//		e_data->add_component(new_object<cBoolDataTracker>(pdata, [](Capture& c, bool v) {
				//			;
				//		}, Capture()));
				//		break;
				//	case FLAME_CHASH("int"):
				//		create_edit<int>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(2+int)"):
				//		create_vec_edit<2, int>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(3+int)"):
				//		create_vec_edit<3, int>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(4+int)"):
				//		create_vec_edit<4, int>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("uint"):
				//		create_edit<uint>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(2+uint)"):
				//		create_vec_edit<2, uint>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(3+uint)"):
				//		create_vec_edit<3, uint>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(4+uint)"):
				//		create_vec_edit<4, uint>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("float"):
				//		create_edit<float>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(2+float)"):
				//		create_vec_edit<2, float>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(3+float)"):
				//		create_vec_edit<3, float>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(4+float)"):
				//		create_vec_edit<4, float>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("uchar"):
				//		create_edit<uchar>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(2+uchar)"):
				//		create_vec_edit<2, uchar>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(3+uchar)"):
				//		create_vec_edit<3, uchar>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::Vec(4+uchar)"):
				//		create_vec_edit<4, uchar>(&setter_capture);
				//		break;
				//	case FLAME_CHASH("flame::StringA"):
				//		ui.e_edit(50.f);

				//		e_data->add_component(new_object<cStringADataTracker>(pdata, [](Capture& c, const char* v) {
				//			;
				//		}, Capture()));
				//		break;
				//	case FLAME_CHASH("flame::StringW"):
				//		ui.e_edit(50.f);

				//		e_data->add_component(new_object<cStringWDataTracker>(pdata, [](Capture& c, const wchar_t* v) {
				//			;
				//		}, Capture()));
				//		break;
				//	}
				//}
				//	break;
				//}

				c_component_tracker->vs[FLAME_HASH(v->name())] = e_data->get_component(cDataTracker);

				ui.e_end_layout();

				ui.e_end_layout();
			}

			ui.e_end_layout();
		}

		ui.e_begin_button_menu(L"Add Component");
		{
			FLAME_SAL(prefix, "Serializer_c");
			std::vector<UdtInfo*> all_udts;
			for (auto i = 0; i < global_db_count(); i++)
			{
				auto db = global_db(i);
				auto udts = db->get_udts();
				for (auto i = 0; i < udts.s; i++)
				{
					auto u = udts.v[i];
					if (std::string(u->name()).compare(0, prefix.l, prefix.s) == 0)
						all_udts.push_back(u);
				}
			}
			std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
				return std::string(a->name()) < std::string(b->name());
			});
			for (auto udt : all_udts)
			{
				ui.e_menu_item(s2w(udt->name() + prefix.l).c_str(), [](Capture& c) {
					looper().add_event([](Capture& c) {
						auto u = c.thiz<UdtInfo>();

						auto dummy = malloc(u->size());
						auto module = u->db()->module();
						{
							auto f = u->find_function("ctor");
							if (f && f->parameter_count() == 0)
								cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
						}
						void* component;
						{
							auto f = u->find_function("create");
							assert(f && check_function(f, "P#flame::Component", {}));
							component = cmf(p2f<MF_vp_v>((char*)module + (uint)f->rva()), dummy);
						}
						scene_editor.selected->add_component((Component*)component);
						{
							auto f = u->find_function("dtor");
							if (f)
								cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
						}
						free(dummy);

						scene_editor.inspector->refresh();
					}, Capture().set_thiz(c.thiz<UdtInfo>()));
				}, Capture().set_thiz(udt));
			}
		}
		ui.e_end_button_menu();
	}
	ui.parents.pop();
}
