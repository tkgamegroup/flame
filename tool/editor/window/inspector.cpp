#include <flame/serialize.h>
#include <flame/universe/ui/utils.h>

#include "../app.h"
#include "../data_tracker.h"
#include "inspector.h"
#include "scene_editor.h"
#include "hierarchy.h"

void begin_item(const wchar_t* title)
{
	ui::e_begin_layout(LayoutVertical, 4.f);
	ui::e_text(title);
	auto e_data = ui::e_empty();
	ui::c_element()->inner_padding_.x() = ui::style_1u(ui::FontSize);
	ui::c_layout(LayoutVertical)->item_padding = 2.f;
	ui::e_end_layout();
	ui::push_parent(e_data);
}

void end_item()
{
	ui::pop_parent();
}

struct cComponentDealer : Component
{
	Component* component;
	void* dummy;
	void* serialize_addr;
	void* unserialize_addr;
	void* dtor_addr;

	cComponentDealer() :
		Component("cComponentDealer")
	{
	}

	~cComponentDealer()
	{
		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), dummy);
		free(dummy);
	}

	void serialize(int offset)
	{
		cmf(p2f<MF_v_vp_u>(serialize_addr), dummy, component, offset);
	}

	void unserialize(int offset)
	{
		cmf(p2f<MF_v_vp_u>(unserialize_addr), dummy, component, offset);
	}
};

template <class T>
void create_edit(void* pdata, cComponentDealer* d, VariableInfo* v)
{
	auto e_edit = create_drag_edit(std::is_floating_point<T>::value);

	struct Capture
	{
		cComponentDealer* d;
		VariableInfo* v;
		cText* drag_text;
	}capture;
	capture.d = d;
	capture.v = v;
	capture.drag_text = e_edit->child(1)->get_component(cText);
	e_edit->child(0)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
		auto& capture = *(Capture*)c;
		if (hash == FLAME_CHASH("text"))
		{
			auto text = ((cText*)t)->text();
			*(T*)((char*)capture.d->dummy + capture.v->offset()) = sto_s<T>(text);
			capture.d->unserialize(capture.v->offset());
			capture.drag_text->set_text(text);
		}
	}, new_mail(&capture));

	auto c_tracker = new_u_object<cDigitalDataTracker<T>>();
	c_tracker->data = pdata;
	ui::current_parent()->add_component(c_tracker);
}

template <uint N, class T>
void create_vec_edit(void* pdata, cComponentDealer* d, VariableInfo* v)
{
	struct Capture
	{
		cComponentDealer* d;
		VariableInfo* v;
		int i;
		cText* drag_text;
	}capture;
	capture.d = d;
	capture.v = v;
	for (auto i = 0; i < N; i++)
	{
		ui::e_begin_layout(LayoutHorizontal, 4.f);
		auto e_edit = create_drag_edit(std::is_floating_point<T>::value);
		capture.i = i;
		capture.drag_text = e_edit->child(1)->get_component(cText);
		e_edit->child(0)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
			auto& capture = *(Capture*)c;
			if (hash == FLAME_CHASH("text"))
			{
				auto text = ((cText*)t)->text();
				(*(Vec<N, T>*)((char*)capture.d->dummy + capture.v->offset()))[capture.i] = sto_s<T>(text);
				capture.d->unserialize(capture.v->offset());
				capture.drag_text->set_text(text);
			}
		}, new_mail(&capture));
		ui::e_text(s2w(Vec<N, T>::coord_name(i)).c_str());
		ui::e_end_layout();
	}

	auto c_tracker = new_u_object<cDigitalVecDataTracker<N, T>>();
	c_tracker->data = pdata;
	ui::current_parent()->add_component(c_tracker);
}

struct cInspectorPrivate : cInspector
{
	void* module;

	~cInspectorPrivate()
	{
		free_module(module);

		editor->inspector = nullptr;
	}

	void refresh()
	{
		auto selected = editor->selected;

		e_layout->remove_child((Entity*)INVALID_POINTER);
		ui::push_parent(e_layout);
		if (!selected)
			ui::e_text(L"Nothing Selected");
		else
		{
			begin_item(L"name");
			auto c_text = ui::e_edit(100.f, s2w(selected->name()).c_str())->get_component(cText)
				->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
				auto editor = *(cSceneEditor**)c;
				if (hash == FLAME_CHASH("text"))
				{
					auto text = ((cText*)t)->text();
					editor->selected->set_name(w2s(text).c_str());
					if (editor->hierarchy)
					{
						auto item = editor->hierarchy->find_item(editor->selected);
						if (item->get_component(cTreeNode))
							item->child(0)->get_component(cText)->set_text(text);
						else
							item->get_component(cText)->set_text(text);
					}
				}
			}, new_mail_p(editor));
			end_item();
			begin_item(L"visible");
			auto checkbox = ui::e_checkbox(L"", selected->visibility_)->get_component(cCheckbox)->
				data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
				if (hash == FLAME_CHASH("checked"))
					(*(Entity**)c)->set_visibility(((cCheckbox*)cb)->checked);
			}, new_mail_p(selected));
			end_item();

			auto components = selected->get_components();
			for (auto i = 0; i < components.s; i++)
			{
				auto component = components.v[i];

				auto udt = find_udt(FLAME_HASH((std::string("D#Serializer_") + component->name).c_str()));

				auto e_component = ui::e_begin_layout(LayoutVertical, 2.f);
				{
					auto c_element = e_component->get_component(cElement);
					c_element->inner_padding_ = Vec4f(4.f);
					c_element->frame_thickness_ = 2.f;
					c_element->frame_color_ = Vec4f(0, 0, 0, 255);
				}
				ui::c_aligner(SizeFitParent, SizeFixed);

				auto c_dealer = new_u_object<cComponentDealer>();
				c_dealer->component = component;
				c_dealer->dummy = malloc(udt->size());
				{
					auto f = udt->find_function("ctor");
					if (f && f->parameter_count() == 0)
						cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), c_dealer->dummy);
				}
				{
					auto f = udt->find_function("serialize");
					assert(f && check_function(f, "D#void", { "P#Component", "D#int" }));
					c_dealer->serialize_addr = (char*)module + (uint)f->rva();
					c_dealer->serialize(-1);
				}
				{
					auto f = udt->find_function("unserialize");
					assert(f && check_function(f, "D#void", { "P#Component", "D#int" }));
					c_dealer->unserialize_addr = (char*)module + (uint)f->rva();
				}
				c_dealer->dtor_addr = nullptr;
				{
					auto f = udt->find_function("dtor");
					if (f)
						c_dealer->dtor_addr = (char*)module + (uint)f->rva();
				}
				e_component->add_component(c_dealer);

				auto e_name = ui::e_text(s2w(component->name).c_str());
				e_name->get_component(cElement)->inner_padding_.z() = 4.f + ui::style_1u(ui::FontSize);
				e_name->get_component(cText)->color = Vec4c(30, 40, 160, 255);
				ui::c_layout();
				ui::push_parent(e_name);
				struct Capture
				{
					Entity* e;
					Component* c;
				}capture;
				capture.e = e_component;
				capture.c = component;
				ui::e_button(Icon_TIMES, [](void* c) {
					auto& capture = *(Capture*)c;
					Capture _capture;
					_capture.e = capture.e;
					_capture.c = capture.c;
					looper().add_event([](void* c) {
						auto& capture = *(Capture*)c;
						capture.e->parent()->remove_child(capture.e);
						capture.c->entity->remove_component(capture.c);
					}, new_mail(&_capture));
				}, new_mail(&capture))
					->get_component(cText)->color = Vec4c(200, 40, 20, 255);
				ui::c_aligner(AlignxRight, AlignyFree);
				ui::pop_parent();

				for (auto i = 0; i < udt->variable_count(); i++)
				{
					auto v = udt->variable(i);
					auto type = v->type();
					auto base_hash = type->base_hash();
					auto pdata = (char*)c_dealer->dummy + v->offset();

					begin_item(s2w(v->name()).c_str());
					auto e_data = ui::current_parent();

					switch (type->tag())
					{
					case TypeEnumSingle:
					{
						auto info = find_enum(base_hash);

						create_enum_combobox(info, 120.f);

						struct Capture
						{
							cComponentDealer* d;
							VariableInfo* v;
							EnumInfo* info;
						}capture;
						capture.d = c_dealer;
						capture.v = v;
						capture.info = info;
						e_data->child(0)->get_component(cCombobox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
							auto& capture = *(Capture*)c;
							if (hash == FLAME_CHASH("index"))
							{
								*(int*)((char*)capture.d->dummy + capture.v->offset()) = capture.info->item(((cCombobox*)cb)->idx)->value();
								capture.d->unserialize(capture.v->offset());
							}
						}, new_mail(&capture));

						auto c_tracker = new_u_object<cEnumSingleDataTracker>();
						c_tracker->data = pdata;
						c_tracker->info = info;
						e_data->add_component(c_tracker);
					}
						break;
					case TypeEnumMulti:
					{
						auto info = find_enum(base_hash);

						create_enum_checkboxs(info);
						for (auto k = 0; k < info->item_count(); k++)
						{
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
								int vl;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							capture.vl = info->item(k)->value();
							e_data->child(k)->child(0)->get_component(cCheckbox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == FLAME_CHASH("checkbox"))
								{
									auto pv = (int*)((char*)capture.d->dummy + capture.v->offset());
									if (((cCheckbox*)cb)->checked)
										(*pv) |= capture.vl;
									else
										(*pv) &= ~capture.vl;
									capture.d->unserialize(capture.v->offset());
								}
							}, new_mail(&capture));
						}

						auto c_tracker = new_u_object<cEnumMultiDataTracker>();
						c_tracker->data = pdata;
						c_tracker->info = info;
						e_data->add_component(c_tracker);
					}
						break;
					case TypeData:
						switch (base_hash)
						{
						case FLAME_CHASH("bool"):
						{
							auto e_checkbox = ui::e_checkbox(L"");
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							e_checkbox->get_component(cCheckbox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == FLAME_CHASH("checkbox"))
								{
									*(bool*)((char*)capture.d->dummy + capture.v->offset()) = ((cCheckbox*)cb)->checked;
									capture.d->unserialize(capture.v->offset());
								}
							}, new_mail(&capture));

							auto c_tracker = new_u_object<cBoolDataTracker>();
							c_tracker->data = pdata;
							e_data->add_component(c_tracker);
						}
							break;
						case FLAME_CHASH("int"):
							create_edit<int>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(2+int)"):
							create_vec_edit<2, int>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(3+int)"):
							create_vec_edit<3, int>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(4+int)"):
							create_vec_edit<4, int>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("uint"):
							create_edit<uint>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(2+uint)"):
							create_vec_edit<2, uint>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(3+uint)"):
							create_vec_edit<3, uint>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(4+uint)"):
							create_vec_edit<4, uint>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("float"):
							create_edit<float>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(2+float)"):
							create_vec_edit<2, float>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(3+float)"):
							create_vec_edit<3, float>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(4+float)"):
							create_vec_edit<4, float>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("uchar"):
							create_edit<uchar>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(2+uchar)"):
							create_vec_edit<2, uchar>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(3+uchar)"):
							create_vec_edit<3, uchar>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("Vec(4+uchar)"):
							create_vec_edit<4, uchar>(pdata, c_dealer, v);
							break;
						case FLAME_CHASH("StringA"):
						{
							auto e_edit = ui::e_edit(50.f);
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == FLAME_CHASH("text"))
								{
									*(StringA*)((char*)capture.d->dummy + capture.v->offset()) = w2s(((cText*)t)->text());
									capture.d->unserialize(capture.v->offset());
								}
							}, new_mail(&capture));

							auto c_tracker = new_u_object<cStringADataTracker>();
							c_tracker->data = pdata;
							e_data->add_component(c_tracker);
						}
							break;
						case FLAME_CHASH("StringW"):
						{
							auto e_edit = ui::e_edit(50.f);
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == FLAME_CHASH("text"))
								{
									*(StringW*)((char*)capture.d->dummy + capture.v->offset()) = ((cText*)t)->text();
									capture.d->unserialize(capture.v->offset());
								}
							}, new_mail(&capture));

							auto c_tracker = new_u_object<cStringWDataTracker>();
							c_tracker->data = pdata;
							e_data->add_component(c_tracker);
						}
							break;
						}
						break;
					}

					end_item();
				}

				ui::e_end_layout();
			}

			ui::e_begin_button_menu(L"Add Component");
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
					struct Capture
					{
						cInspectorPrivate* i;
						UdtInfo* u;
					}capture;
					capture.i = this;
					capture.u = udt;
					ui::e_menu_item(s2w(udt->type()->name() + prefix.l).c_str(), [](void* c) {
						auto& capture = *(Capture*)c;
						looper().add_event([](void* c) {
							auto& capture = *(Capture*)c;

							auto dummy = malloc(capture.u->size());
							auto module = load_module(capture.u->db()->module_name());
							{
								auto f = capture.u->find_function("ctor");
								if (f && f->parameter_count() == 0)
									cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
							}
							void* component;
							{
								auto f = capture.u->find_function("create");
								assert(f && check_function(f, "P#Component", {}));
								component = cmf(p2f<MF_vp_v>((char*)module + (uint)f->rva()), dummy);
							}
							capture.i->editor->selected->add_component((Component*)component);
							{
								auto f = capture.u->find_function("dtor");
								if (f)
									cmf(p2f<MF_v_v>((char*)module + (uint)f->rva()), dummy);
							}
							free_module(module);
							free(dummy);

							capture.i->refresh();
						}, new_mail(&capture));
					}, new_mail(&capture));
				}
			}
			ui::e_end_button_menu();
		}
		ui::pop_parent();
	}
};

void cInspector::update_data_tracker(uint component_hash, uint data_offset) const
{
	if (!editor->selected)
		return;
	for (auto i = 2; i < e_layout->child_count(); i++)
	{
		auto e_component = e_layout->child(i);
		auto dealer = e_component->get_component(cComponentDealer);
		if (dealer && dealer->component->name_hash == component_hash)
		{
			for (auto j = 1; j < e_component->child_count(); j++)
			{
				auto e_data = e_component->child(j)->child(1);
				auto tracker = e_data->get_component(cDataTracker);
				if (tracker && tracker->data == (char*)dealer->dummy + data_offset)
				{
					dealer->serialize(data_offset);
					tracker->update_view();
				}
			}
		}
	}
}

void cInspector::refresh()
{
	((cInspectorPrivate*)this)->refresh();
}

void open_inspector(cSceneEditor* editor, const Vec2f& pos)
{
	ui::push_parent(app.root);
	ui::next_element_pos = pos;
	ui::next_element_size = Vec2f(200.f, 900.f);
	ui::e_begin_docker_floating_container();
	ui::e_begin_docker();
	auto c_tab = ui::e_begin_docker_page(L"Inspector").tab->get_component(cDockerTab);
	{
		ui::current_entity()->get_component(cElement)->inner_padding_ = Vec4f(4.f);
		auto c_layout = ui::c_layout(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
	}
	auto c_inspector = new_u_object<cInspectorPrivate>();
	ui::current_entity()->add_component(c_inspector);
	c_inspector->tab = c_tab;
	c_inspector->editor = editor;
	c_inspector->module = load_module(L"flame_universe.dll");
	editor->inspector = c_inspector;

	ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f));
	c_inspector->e_layout = ui::e_empty();
	{
		ui::c_element()->clip_children = true;
		auto cl = ui::c_layout(LayoutVertical);
		cl->item_padding = 4.f;
		cl->width_fit_children = false;
		cl->height_fit_children = false;
		ui::c_aligner(SizeFitParent, SizeFitParent);
	}
	ui::e_end_scroll_view1();

	c_inspector->refresh();

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_floating_container();
	ui::pop_parent();
}
