#include <flame/universe/utils/typeinfo.h>
#include "app.h"

void begin_item(const wchar_t* title)
{
	utils::e_begin_layout(LayoutVertical, 4.f);
	utils::e_text(title);
	auto e_data = utils::e_empty();
	utils::c_element()->padding.x() = utils::style_1u(utils::FontSize);
	utils::c_layout(LayoutVertical)->item_padding = 2.f;
	utils::e_end_layout();
	utils::push_parent(e_data);
}

void end_item()
{
	utils::pop_parent();
}

struct Setter
{
	void* addr;

	void unserialize(int offset)
	{
		//cmf(p2f<MF_v_vp_u>(unserialize_addr), );
	}
};

template <class T>
void create_edit(void* pdata, VariableInfo* v)
{
	utils::e_drag_edit();

	utils::current_parent()->add_component(new_object<cDigitalDataTracker<T>>(pdata, [](void* c, T v, bool exit_editing) {
		;
	}, Mail::from_p(v)));
}

template <uint N, class T>
void create_vec_edit(void* pdata, VariableInfo* v)
{
	for (auto i = 0; i < N; i++)
	{
		utils::e_begin_layout(LayoutHorizontal, 4.f);
		utils::e_drag_edit();
		utils::e_text(s2w(Vec<N, T>::coord_name(i)).c_str());
		utils::e_end_layout();
	}

	utils::current_parent()->add_component(new_object<cDigitalVecDataTracker<N, T>>(pdata, [](void* c, const Vec<N, T>& v, bool exit_editing) {
		;
	}, Mail::from_p(v)));
}

cInspector::cInspector() :
	Component("cInspector")
{
	auto e_page = utils::e_begin_docker_page(L"Inspector").second;
	{
		utils::current_entity()->get_component(cElement)->padding = Vec4f(4.f);
		auto c_layout = utils::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;

		e_page->add_component(this);
	}

	module = load_module(L"flame_universe.dll");

	utils::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f));
		e_layout = utils::e_empty();
		{
			utils::c_element()->clip_flags = ClipChildren;
			auto cl = utils::c_layout(LayoutVertical);
			cl->item_padding = 4.f;
			cl->width_fit_children = false;
			cl->height_fit_children = false;
			utils::c_aligner(SizeFitParent, SizeFitParent);
		}
	utils::e_end_scroll_view1();

	refresh();
}

cInspector::~cInspector()
{
	free_module(module);

	app.inspector = nullptr;
}

//void cInspector::update_data_tracker(uint component_hash, uint data_offset) const
//{
//	if (!app.selected)
//		return;
//	for (auto i = 2; i < e_layout->child_count(); i++)
//	{
//		auto e_component = e_layout->child(i);
//		auto dealer = e_component->get_component(cComponentDealer);
//		if (dealer && dealer->component->name_hash == component_hash)
//		{
//			for (auto j = 1; j < e_component->child_count(); j++)
//			{
//				auto e_data = e_component->child(j)->child(1);
//				auto tracker = e_data->get_component(cDataTracker);
//				if (tracker && tracker->data == (char*)dealer->dummy + data_offset)
//				{
//					dealer->serialize(data_offset);
//					tracker->update_view();
//				}
//			}
//		}
//	}
//}

void cInspector::refresh()
{
	e_layout->remove_children(0, -1);

	utils::push_parent(e_layout);
	if (!app.selected)
		utils::e_text(L"Nothing Selected");
	else
	{
		begin_item(L"name");
		auto c_text = utils::e_edit(100.f, s2w(app.selected->name()).c_str())->get_component(cText);
		c_text->data_changed_listeners.add([](void* c, uint hash, void*) {
			if (hash == FLAME_CHASH("text"))
			{
				auto text = (*(cText**)c)->text();
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
		}, Mail::from_p(c_text));
		end_item();
		begin_item(L"visible");
		auto checkbox = utils::e_checkbox(L"", app.selected->visible_)->get_component(cCheckbox);
		checkbox->data_changed_listeners.add([](void* c, uint hash, void*) {
			if (hash == FLAME_CHASH("checked"))
				app.selected->set_visible((*(cCheckbox**)c)->checked);
			return true;
		}, Mail::from_p(checkbox));
		end_item();

		auto components = app.selected->get_components();
		for (auto i = 0; i < components.s; i++)
		{
			auto component = components.v[i];

			auto udt = find_udt(FLAME_HASH((std::string("D#flame::") + component->name).c_str()));

			auto e_component = utils::e_begin_layout(LayoutVertical, 2.f);
			{
				auto c_element = e_component->get_component(cElement);
				c_element->padding = Vec4f(4.f);
				c_element->frame_thickness = 2.f;
				c_element->frame_color = Vec4f(0, 0, 0, 255);
			}
			utils::c_aligner(SizeFitParent, SizeFixed);

			auto e_name = utils::e_text(s2w(component->name).c_str());
			e_name->get_component(cElement)->padding.z() = 4.f + utils::style_1u(utils::FontSize);
			e_name->get_component(cText)->color_ = Vec4c(30, 40, 160, 255);
			utils::c_layout();
			utils::push_parent(e_name);
			struct Capture
			{
				Entity* e;
				Component* c;
			}capture;
			capture.e = e_component;
			capture.c = component;
			utils::e_button(Icon_TIMES, [](void* c) {
				auto& capture = *(Capture*)c;
				Capture _capture;
				_capture.e = capture.e;
				_capture.c = capture.c;
				looper().add_event([](void* c, bool*) {
					auto& capture = *(Capture*)c;
					capture.e->parent()->remove_child(capture.e);
					capture.c->entity->remove_component(capture.c);
				}, Mail::from_t(&_capture));
			}, Mail::from_t(&capture))
				->get_component(cText)->color_ = Vec4c(200, 40, 20, 255);
			utils::c_aligner(AlignxRight, AlignyFree);
			utils::pop_parent();

			for (auto i = 0; i < udt->variable_count(); i++)
			{
				auto v = udt->variable(i);
				auto type = v->type();
				auto base_hash = type->base_hash();
				auto pdata = (char*)component + v->offset();

				begin_item(s2w(v->name()).c_str());
				auto e_data = utils::current_parent();

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
					switch (base_hash)
					{
					case FLAME_CHASH("bool"):
						utils::e_checkbox(L"");

						e_data->add_component(new_object<cBoolDataTracker>(pdata, [](void* c, bool v) {
							;
						}, Mail::from_p(nullptr)));
						break;
					case FLAME_CHASH("int"):
						create_edit<int>(pdata, v);
						break;
					case FLAME_CHASH("Vec(2+int)"):
						create_vec_edit<2, int>(pdata, v);
						break;
					case FLAME_CHASH("Vec(3+int)"):
						create_vec_edit<3, int>(pdata, v);
						break;
					case FLAME_CHASH("Vec(4+int)"):
						create_vec_edit<4, int>(pdata, v);
						break;
					case FLAME_CHASH("uint"):
						create_edit<uint>(pdata, v);
						break;
					case FLAME_CHASH("Vec(2+uint)"):
						create_vec_edit<2, uint>(pdata, v);
						break;
					case FLAME_CHASH("Vec(3+uint)"):
						create_vec_edit<3, uint>(pdata, v);
						break;
					case FLAME_CHASH("Vec(4+uint)"):
						create_vec_edit<4, uint>(pdata, v);
						break;
					case FLAME_CHASH("float"):
						create_edit<float>(pdata, v);
						break;
					case FLAME_CHASH("Vec(2+float)"):
						create_vec_edit<2, float>(pdata, v);
						break;
					case FLAME_CHASH("Vec(3+float)"):
						create_vec_edit<3, float>(pdata, v);
						break;
					case FLAME_CHASH("Vec(4+float)"):
						create_vec_edit<4, float>(pdata, v);
						break;
					case FLAME_CHASH("uchar"):
						create_edit<uchar>(pdata, v);
						break;
					case FLAME_CHASH("Vec(2+uchar)"):
						create_vec_edit<2, uchar>(pdata, v);
						break;
					case FLAME_CHASH("Vec(3+uchar)"):
						create_vec_edit<3, uchar>(pdata, v);
						break;
					case FLAME_CHASH("Vec(4+uchar)"):
						create_vec_edit<4, uchar>(pdata, v);
						break;
					case FLAME_CHASH("StringA"):
						utils::e_edit(50.f);

						e_data->add_component(new_object<cStringADataTracker>(pdata, [](void* c, const char* v) {
							;
						}, Mail::from_p(nullptr)));
						break;
					case FLAME_CHASH("StringW"):
						utils::e_edit(50.f);

						e_data->add_component(new_object<cStringWDataTracker>(pdata, [](void* c, const wchar_t* v) {
							;
						}, Mail::from_p(nullptr)));
						break;
					}
					break;
				}

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
					if (std::string(u->type()->name()).compare(0, prefix.l, prefix.s) == 0)
						all_udts.push_back(u);
				}
			}
			std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
				return std::string(a->type()->name()) < std::string(b->type()->name());
			});
			for (auto udt : all_udts)
			{
				utils::e_menu_item(s2w(udt->type()->name() + prefix.l).c_str(), [](void* c) {
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
