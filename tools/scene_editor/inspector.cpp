#include <flame/universe/utils/typeinfo.h>
#include "app.h"

void begin_item(const wchar_t* title)
{
	utils::e_begin_layout(LayoutVertical, 4.f);
	utils::e_text(title);
	auto e_data = utils::e_empty();
	utils::c_element()->padding.x() = utils::style(FontSize).u.x();
	utils::c_layout(LayoutVertical)->item_padding = 2.f;
	utils::e_end_layout();
	utils::push_parent(e_data);
}

void end_item()
{
	utils::pop_parent();
}

struct SetterCapture
{
	void* p;
	void* o;
	void* f;
};

template <class T>
void create_edit(SetterCapture* c)
{
	utils::e_drag_edit();

	utils::current_parent()->add_component(new_object<cDigitalDataTracker<T>>(c->p, [](void* c, T v, bool exit_editing) {
		auto& capture = *(SetterCapture*)c;
		if (capture.f)
			Setter_t<T>::set_s(capture.o, capture.f, v, app.inspector);
		else
			*(T*)capture.p = v;
	}, Mail::from_t(c)));
}

template <uint N, class T>
void create_vec_edit(SetterCapture* c)
{
	for (auto i = 0; i < N; i++)
		utils::e_drag_edit();

	auto p = utils::current_parent();
	p->get_component(cLayout)->type = LayoutHorizontal;
	p->add_component(new_object<cDigitalVecDataTracker<N, T>>(c->p, [](void* c, const Vec<N, T>& v, bool exit_editing) {
		auto& capture = *(SetterCapture*)c;
		if (capture.f)
			Setter_t<Vec<N, T>>::set_s(capture.o, capture.f, (Vec<N, T>*)&v, app.inspector);
		else
			*(Vec<N, T>*)capture.p = v;
	}, Mail::from_t(c)));
}

cInspector::cInspector() :
	Component("cInspector")
{
	utils::next_element_padding = 4.f;
	auto e_page = utils::e_begin_docker_page(L"Inspector").second;
	{
		auto c_layout = utils::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

	utils::e_begin_scrollbar(ScrollbarVertical, true);
		e_layout = utils::e_empty();
		{
			utils::c_element()->clip_flags = ClipChildren;
			auto cl = utils::c_layout(LayoutVertical);
			cl->item_padding = 4.f;
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			utils::c_aligner(AlignMinMax, AlignMinMax);
		}
	utils::e_end_scrollbar();

	refresh();
}

cInspector::~cInspector()
{
	app.inspector = nullptr;
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
		l = t->data_changed_listeners.add([](void* c, uint hash, void* sender) {
			if (sender == app.inspector)
				return true;
			auto thiz = *(cComponentTracker**)c;
			auto it = thiz->vs.find(hash);
			if (it != thiz->vs.end())
				it->second->update_view();
			return true;
		}, Mail::from_p(this));
	}

	~cComponentTracker()
	{
		t->data_changed_listeners.remove(l);
	}
};

void cInspector::refresh()
{
	e_layout->remove_children(0, -1);

	utils::push_parent(e_layout);
	if (!app.selected)
		utils::e_text(L"Nothing Selected");
	else
	{
		begin_item(L"name");
		utils::e_edit(100.f, s2w(app.selected->name()).c_str())->get_component(cText)->data_changed_listeners.add([](void*, uint hash, void*) {
			if (hash == FLAME_CHASH("text"))
			{
				auto text = ((cText*)Component::current())->text.v;
				app.selected->set_name(w2s(text).c_str());
				if (app.hierarchy)
				{
					auto item = app.hierarchy->find_item(app.selected);
					if (item->get_component(cTreeNode))
						item->child(0)->get_component(cText)->set_text(text);
					else
						item->get_component(cText)->set_text(text);
				}
			}
			return true;
		}, Mail());
		end_item();
		begin_item(L"visible");
		utils::e_checkbox(L"", app.selected->visible_)->get_component(cCheckbox)->data_changed_listeners.add([](void*, uint hash, void*) {
			if (hash == FLAME_CHASH("checked"))
				app.selected->set_visible(((cCheckbox*)Component::current())->checked);
			return true;
		}, Mail());
		end_item();

		auto components = app.selected->get_components();
		for (auto i = 0; i < components.s; i++)
		{
			auto component = components.v[i];

			auto udt = find_udt(FLAME_HASH((std::string("flame::") + component->name).c_str()));
			auto module = udt->db()->module();

			utils::next_element_padding = 4.f;
			auto e_component = utils::e_begin_layout(LayoutVertical, 2.f);
			utils::c_aligner(AlignMinMax, 0);

			auto c_component_tracker = new cComponentTracker(component);
			c_component_tracker->id = FLAME_HASH(component->name);
			e_component->add_component(c_component_tracker);

			utils::e_begin_layout(LayoutHorizontal, 4.f);
			utils::e_text(s2w(component->name).c_str())->get_component(cText)->color = Vec4c(30, 40, 160, 255);
				struct Capture
				{
					Entity* e;
					Component* c;
				}capture;
				capture.e = e_component;
				capture.c = component;
				utils::push_style(ButtonColorNormal, common(Vec4c(0)));
				utils::push_style(ButtonColorHovering, common(utils::style(FrameColorHovering).c));
				utils::push_style(ButtonColorActive, common(utils::style(FrameColorActive).c));
				utils::e_button(L"X", [](void* c) {
					auto& capture = *(Capture*)c;
					Capture _capture;
					_capture.e = capture.e;
					_capture.c = capture.c;
					looper().add_event([](void* c, bool*) {
						auto& capture = *(Capture*)c;
						capture.e->parent()->remove_child(capture.e);
						capture.c->entity->remove_component(capture.c);
					}, Mail::from_t(&_capture));
				}, Mail::from_t(&capture));
				utils::pop_style(ButtonColorNormal);
				utils::pop_style(ButtonColorHovering);
				utils::pop_style(ButtonColorActive);
			utils::e_end_layout();

			for (auto i = 0; i < udt->variable_count(); i++)
			{
				auto v = udt->variable(i);
				auto type = v->type();
				auto base_hash = type->base_hash();
				auto pdata = (char*)component + v->offset();

				begin_item(s2w(v->name()).c_str());
				auto e_data = utils::current_parent();

				auto f_set = udt->find_function((std::string("set_") + v->name()).c_str());
				auto f_set_addr = f_set ? (char*)module + (uint)f_set->rva() : nullptr;

				switch (type->tag())
				{
				case TypeEnumSingle:
				{
					auto info = find_enum(base_hash);
					utils::create_enum_combobox(info);

					e_data->add_component(new_object<cEnumSingleDataTracker>(pdata, info, [](void* c, int v) {
						;
					}, Mail::from_p(nullptr)));
				}
				break;
				case TypeEnumMulti:
				{
					auto info = find_enum(base_hash);
					utils::create_enum_checkboxs(info);

					e_data->add_component(new_object<cEnumMultiDataTracker>(pdata, info, [](void* c, int v) {
						;
					}, Mail::from_p(nullptr)));
				}
				break;
				case TypeData:
				{
					SetterCapture setter_capture;
					setter_capture.p = pdata;
					setter_capture.o = component;
					setter_capture.f = f_set_addr;
					switch (base_hash)
					{
					case FLAME_CHASH("bool"):
						utils::e_checkbox(L"");

						e_data->add_component(new_object<cBoolDataTracker>(pdata, [](void* c, bool v) {
							;
						}, Mail::from_p(nullptr)));
						break;
					case FLAME_CHASH("int"):
						create_edit<int>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(2+int)"):
						create_vec_edit<2, int>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(3+int)"):
						create_vec_edit<3, int>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(4+int)"):
						create_vec_edit<4, int>(&setter_capture);
						break;
					case FLAME_CHASH("uint"):
						create_edit<uint>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(2+uint)"):
						create_vec_edit<2, uint>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(3+uint)"):
						create_vec_edit<3, uint>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(4+uint)"):
						create_vec_edit<4, uint>(&setter_capture);
						break;
					case FLAME_CHASH("float"):
						create_edit<float>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(2+float)"):
						create_vec_edit<2, float>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(3+float)"):
						create_vec_edit<3, float>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(4+float)"):
						create_vec_edit<4, float>(&setter_capture);
						break;
					case FLAME_CHASH("uchar"):
						create_edit<uchar>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(2+uchar)"):
						create_vec_edit<2, uchar>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(3+uchar)"):
						create_vec_edit<3, uchar>(&setter_capture);
						break;
					case FLAME_CHASH("flame::Vec(4+uchar)"):
						create_vec_edit<4, uchar>(&setter_capture);
						break;
					case FLAME_CHASH("flame::StringA"):
						utils::e_edit(50.f);

						e_data->add_component(new_object<cStringADataTracker>(pdata, [](void* c, const char* v) {
							;
						}, Mail::from_p(nullptr)));
						break;
					case FLAME_CHASH("flame::StringW"):
						utils::e_edit(50.f);

						e_data->add_component(new_object<cStringWDataTracker>(pdata, [](void* c, const wchar_t* v) {
							;
						}, Mail::from_p(nullptr)));
						break;
					}
				}
					break;
				}

				c_component_tracker->vs[FLAME_HASH(v->name())] = e_data->get_component(cDataTracker);

				end_item();
			}

			utils::e_end_layout();
		}

		utils::e_begin_button_menu(L"Add Component");
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
				utils::e_menu_item(s2w(udt->name() + prefix.l).c_str(), [](void* c) {
					looper().add_event([](void* c, bool*) {
						auto u = *(UdtInfo**)c;

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
						app.selected->add_component((Component*)component);
						{
							auto f = u->find_function("dtor");
							if (f)
								cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
						}
						free(dummy);

						app.inspector->refresh();
					}, Mail::from_p(*(UdtInfo**)c));
				}, Mail::from_p(udt));
			}
		}
		utils::e_end_button_menu();
	}
	utils::pop_parent();
}
