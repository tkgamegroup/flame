#include <flame/graphics/font.h>
#include <flame/foundation/serialize.h>
#include <flame/universe/topmost.h>
#include <flame/universe/default_style.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "../data_tracker.h"
#include "inspector.h"
#include "scene_editor.h"
#include "hierarchy.h"

Entity* create_item(const std::wstring& title)
{
	auto e_item = Entity::create();
	{
		e_item->add_component(cElement::create());

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 4.f;
		e_item->add_component(c_layout);
	}

	auto e_title = Entity::create();
	e_item->add_child(e_title);
	{
		e_title->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_pixel);
		c_text->set_text(title);
		e_title->add_component(c_text);
	}

	auto e_data = Entity::create();
	e_item->add_child(e_data);
	{
		auto c_element = cElement::create();
		c_element->inner_padding_ = Vec4f(default_style.font_size, 0.f, 0.f, 0.f);
		e_data->add_component(c_element);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 2.f;
		e_data->add_component(c_layout);
	}

	return e_item;
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

template<class T>
void create_edit(Entity* parent, void* pdata, cComponentDealer* d, VariableInfo* v)
{
	auto e_edit = create_drag_edit(app.font_atlas_pixel, 1.f, std::is_floating_point<T>::value);
	parent->add_child(e_edit);

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
		if (hash == cH("text"))
		{
			auto& text = ((cText*)t)->text();
			*(T*)((char*)capture.d->dummy + capture.v->offset()) = sto_s<T>(text.c_str());
			capture.d->unserialize(capture.v->offset());
			capture.drag_text->set_text(text);
		}
	}, new_mail(&capture));

	auto c_tracker = new_u_object<cDigitalDataTracker<T>>();
	c_tracker->data = pdata;
	parent->add_component(c_tracker);
}

template<uint N, class T>
void create_vec_edit(Entity* parent, void* pdata, cComponentDealer* d, VariableInfo* v)
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
		auto e_edit = create_drag_edit(app.font_atlas_pixel, 1.f, std::is_floating_point<T>::value);
		parent->add_child(wrap_standard_text(e_edit, false, app.font_atlas_pixel, 1.f, s2w(Vec<N, T>::coord_name(i))));
		capture.i = i;
		capture.drag_text = e_edit->child(1)->get_component(cText);
		e_edit->child(0)->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
			auto& capture = *(Capture*)c;
			if (hash == cH("text"))
			{
				auto& text = ((cText*)t)->text();
				(*(Vec<N, T>*)((char*)capture.d->dummy + capture.v->offset()))[capture.i] = sto_s<T>(text.c_str());
				capture.d->unserialize(capture.v->offset());
				capture.drag_text->set_text(text);
			}
		}, new_mail(&capture));
	}

	auto c_tracker = new_u_object<cDigitalVecDataTracker<N, T>>();
	c_tracker->data = pdata;
	parent->add_component(c_tracker);
}

struct cInspectorPrivate : cInspector
{
	void* module;

	Entity* e_add_component_menu;

	~cInspectorPrivate()
	{
		free_module(module);

		editor->inspector = nullptr;
	}

	void refresh()
	{
		auto selected = editor->selected;

		e_layout->remove_child((Entity*)FLAME_INVALID_POINTER);
		if (!selected)
		{
			auto e_text = Entity::create();
			e_layout->add_child(e_text);
			{
				e_text->add_component(cElement::create());

				auto c_text = cText::create(app.font_atlas_pixel);
				c_text->set_text(L"Nothing Selected");
				e_text->add_component(c_text);
			}
		}
		else
		{
			{
				auto e_item = create_item(L"name");
				e_layout->add_child(e_item);

				auto e_edit = create_standard_edit(100.f, app.font_atlas_pixel, 1.f);
				e_item->child(1)->add_child(e_edit);
				auto c_text = e_edit->get_component(cText);
				c_text->set_text(s2w(selected->name()));
				c_text->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
					auto editor = *(cSceneEditor**)c;
					if (hash == cH("text"))
					{
						auto& text = ((cText*)t)->text();
						editor->selected->set_name(w2s(text));
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
			}
			{
				auto e_item = create_item(L"visible");
				e_layout->add_child(e_item);

				auto e_checkbox = create_standard_checkbox();
				e_item->child(1)->add_child(e_checkbox);
				auto checkbox = e_checkbox->get_component(cCheckbox);
				checkbox->set_checked(selected->visibility_, false);
				checkbox->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
					if (hash == cH("checked"))
						(*(Entity**)c)->set_visibility(((cCheckbox*)cb)->checked);
				}, new_mail_p(selected));
			}

			auto components = selected->get_components();
			for (auto component : *components.p)
			{
				auto e_component = Entity::create();
				e_layout->add_child(e_component);
				{
					auto c_element = cElement::create();
					c_element->inner_padding_ = Vec4f(4.f);
					c_element->frame_thickness_ = 2.f;
					c_element->frame_color_ = Vec4f(0, 0, 0, 255);
					e_component->add_component(c_element);

					auto c_aligner = cAligner::create();
					c_aligner->width_policy_ = SizeFitParent;
					e_component->add_component(c_aligner);

					auto c_layout = cLayout::create(LayoutVertical);
					c_layout->item_padding = 2.f;
					e_component->add_component(c_layout);
				}

				auto udt = find_udt(app.dbs, H((std::string("Serializer_") + component->name).c_str()));

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
					assert(f && f->return_type().hash == TypeInfo(TypeData, "void").hash && f->parameter_count() == 2 && f->parameter_type(0).hash == TypeInfo(TypePointer, "Component").hash && f->parameter_type(1).hash == TypeInfo(TypeData, "int").hash);
					c_dealer->serialize_addr = (char*)module + (uint)f->rva();
					c_dealer->serialize(-1);
				}
				{
					auto f = udt->find_function("unserialize");
					assert(f && f->return_type().hash == TypeInfo(TypeData, "void").hash && f->parameter_count() == 2 && f->parameter_type(0).hash == TypeInfo(TypePointer, "Component").hash && f->parameter_type(1).hash == TypeInfo(TypeData, "int").hash);
					c_dealer->unserialize_addr = (char*)module + (uint)f->rva();
				}
				c_dealer->dtor_addr = nullptr;
				{
					auto f = udt->find_function("dtor");
					if (f)
						c_dealer->dtor_addr = (char*)module + (uint)f->rva();
				}
				e_component->add_component(c_dealer);

				auto e_name = Entity::create();
				e_component->add_child(e_name);
				{
					auto c_element = cElement::create();
					c_element->inner_padding_ = Vec4f(0.f, 0.f, 4.f + default_style.font_size, 0.f);
					e_name->add_component(c_element);

					auto c_text = cText::create(app.font_atlas_pixel);
					c_text->color = Vec4c(30, 40, 160, 255);
					c_text->set_text(s2w(component->name));
					e_name->add_component(c_text);

					e_name->add_component(cLayout::create(LayoutFree));
				}

				auto e_close = Entity::create();
				e_name->add_child(e_close);
				{
					e_close->add_component(cElement::create());

					auto c_text = cText::create(app.font_atlas_pixel);
					c_text->color = Vec4c(200, 40, 20, 255);
					c_text->set_text(Icon_TIMES);
					e_close->add_component(c_text);

					auto c_event_receiver = cEventReceiver::create();
					struct Capture
					{
						Entity* e;
						Component* c;
					}capture;
					capture.e = e_component;
					capture.c = component;
					c_event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
						auto& capture = *(Capture*)c;

						if (is_mouse_clicked(action, key))
						{
							Capture _capture;
							_capture.e = capture.e;
							_capture.c = capture.c;
							looper().add_event([](void* c) {
								auto& capture = *(Capture*)c;
								capture.e->parent()->remove_child(capture.e);
								capture.c->entity->remove_component(capture.c);
							}, new_mail(&_capture));
						}
					}, new_mail(&capture));
					e_close->add_component(c_event_receiver);

					auto c_aligner = cAligner::create();
					c_aligner->x_align_ = AlignxRight;
					e_close->add_component(c_aligner);
				}

				for (auto j = 0; j < udt->variable_count(); j++)
				{
					auto v = udt->variable(j);
					auto pdata = (char*)c_dealer->dummy + v->offset();

					auto e_item = create_item(s2w(v->name()));
					e_component->add_child(e_item);
					auto e_data = e_item->child(1);
					switch (v->type().tag)
					{
					case TypeEnumSingle:
					{
						auto info = find_enum(app.dbs, v->type().hash);

						create_enum_combobox(info, 120.f, app.font_atlas_pixel, 1.f, e_data);

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
							if (hash == cH("index"))
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
						auto info = find_enum(app.dbs, v->type().hash);

						create_enum_checkboxs(info, app.font_atlas_pixel, 1.f, e_data);
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
								if (hash == cH("checkbox"))
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
						switch (v->type().hash)
						{
						case cH("bool"):
						{
							auto e_checkbox = create_standard_checkbox();
							e_data->add_child(e_checkbox);
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							e_checkbox->get_component(cCheckbox)->data_changed_listeners.add([](void* c, Component* cb, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == cH("checkbox"))
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
						case cH("int"):
							create_edit<int>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(2+int)"):
							create_vec_edit<2, int>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(3+int)"):
							create_vec_edit<3, int>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(4+int)"):
							create_vec_edit<4, int>(e_data, pdata, c_dealer, v);
							break;
						case cH("uint"):
							create_edit<uint>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(2+uint)"):
							create_vec_edit<2, uint>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(3+uint)"):
							create_vec_edit<3, uint>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(4+uint)"):
							create_vec_edit<4, uint>(e_data, pdata, c_dealer, v);
							break;
						case cH("float"):
							create_edit<float>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(2+float)"):
							create_vec_edit<2, float>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(3+float)"):
							create_vec_edit<3, float>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(4+float)"):
							create_vec_edit<4, float>(e_data, pdata, c_dealer, v);
							break;
						case cH("uchar"):
							create_edit<uchar>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(2+uchar)"):
							create_vec_edit<2, uchar>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(3+uchar)"):
							create_vec_edit<3, uchar>(e_data, pdata, c_dealer, v);
							break;
						case cH("Vec(4+uchar)"):
							create_vec_edit<4, uchar>(e_data, pdata, c_dealer, v);
							break;
						case cH("std::basic_string(char)"):
						{
							auto e_edit = create_standard_edit(50.f, app.font_atlas_pixel, 1.f);
							e_data->add_child(e_edit);
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == cH("text"))
								{
									*(std::string*)((char*)capture.d->dummy + capture.v->offset()) = w2s(((cText*)t)->text());
									capture.d->unserialize(capture.v->offset());
								}
							}, new_mail(&capture));

							auto c_tracker = new_u_object<cStringDataTracker>();
							c_tracker->data = pdata;
							e_data->add_component(c_tracker);
						}
							break;
						case cH("std::basic_string(wchar_t)"):
						{
							auto e_edit = create_standard_edit(50.f, app.font_atlas_pixel, 1.f);
							e_data->add_child(e_edit);
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							e_edit->get_component(cText)->data_changed_listeners.add([](void* c, Component* t, uint hash, void*) {
								auto& capture = *(Capture*)c;
								if (hash == cH("text"))
								{
									*(std::wstring*)((char*)capture.d->dummy + capture.v->offset()) = ((cText*)t)->text();
									capture.d->unserialize(capture.v->offset());
								}
							}, new_mail(&capture));

							auto c_tracker = new_u_object<cWStringDataTracker>();
							c_tracker->data = pdata;
							e_data->add_component(c_tracker);
						}
							break;
						}
						break;
					}
				}
			}
			delete_mail(components);

			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add Component", app.root, e_add_component_menu, true, SideS, false, false, false, nullptr);
			e_layout->add_child(e_menu_btn);
		}
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
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = e_container->get_component(cElement);
		c_element->pos_ = pos;
		c_element->size_.x() = 200.f;
		c_element->size_.y() = 900.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker, 0);

	auto tab = create_standard_docker_tab(app.font_atlas_pixel, L"Inspector", app.root);
	e_docker->child(0)->add_child(tab);

	auto e_page = get_docker_page_model()->copy();
	{
		e_page->get_component(cElement)->inner_padding_ = Vec4f(4.f);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_inspector = new_u_object<cInspectorPrivate>();
	e_page->add_component(c_inspector);
	c_inspector->tab = tab->get_component(cDockerTab);
	c_inspector->editor = editor;
	c_inspector->module = load_module(L"flame_universe.dll");

	{
		auto e_menu = create_standard_menu();
		c_inspector->e_add_component_menu = e_menu;

		#define COMPONENT_PREFIX "Serializer_c"
		std::vector<UdtInfo*> all_udts;
		for (auto db : app.dbs)
		{
			auto udts = db->get_udts();
			for (auto i = 0; i < udts.p->size(); i++)
			{
				auto u = udts.p->at(i);
				if (u->type().name.compare(0, strlen(COMPONENT_PREFIX), COMPONENT_PREFIX) == 0)
					all_udts.push_back(u);
			}
			delete_mail(udts);
		}
		std::sort(all_udts.begin(), all_udts.end(), [](UdtInfo* a, UdtInfo* b) {
			return a->type().name < b->type().name;
		});
		for (auto udt : all_udts)
		{
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(udt->type().name.c_str() + strlen(COMPONENT_PREFIX)));
			e_menu->add_child(e_item);
			struct Capture
			{
				cInspectorPrivate* i;
				UdtInfo* u;
			}capture;
			capture.i = c_inspector;
			capture.u = udt;
			e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				auto& capture = *(Capture*)c;
				if (is_mouse_clicked(action, key))
				{
					destroy_topmost(app.root);

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
							assert(f && f->return_type().hash == TypeInfo(TypePointer, "Component").hash && f->parameter_count() == 0);
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
				}
			}, new_mail(&capture));
		}
		#undef COMPONENT_PREFIX
	}

	editor->inspector = c_inspector;

	auto e_layout = Entity::create();
	{
		auto c_element = cElement::create();
		c_element->clip_children = true;
		e_layout->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy_ = SizeFitParent;
		c_aligner->height_policy_ = SizeFitParent;
		e_layout->add_component(c_aligner);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_layout->add_component(c_layout);
	}

	c_inspector->e_layout = e_layout;

	e_page->add_child(wrap_standard_scrollbar(e_layout, ScrollbarVertical, true, 1.f));

	c_inspector->refresh();
}
