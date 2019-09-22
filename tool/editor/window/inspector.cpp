#include <flame/graphics/font.h>
#include <flame/foundation/serialize.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "inspector.h"
#include "scene_editor.h"

Entity* create_item(const std::wstring& title)
{
	auto e_item = Entity::create();
	{
		e_item->add_component(cElement::create());

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 2.f;
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
		c_element->inner_padding = Vec4f(app.font_atlas_pixel->pixel_height, 0.f, 0.f, 0.f);
		e_data->add_component(c_element);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 2.f;
		e_data->add_component(c_layout);
	}

	return e_item;
}

struct cComponentDealer : Component
{
	Component* component;
	void* dummy;
	void* dtor_addr;
	void* data_changed_addr;

	cComponentDealer() :
		Component("ComponentDealer")
	{
	}

	~cComponentDealer()
	{
		if (dtor_addr)
			cmf(p2f<MF_v_v>(dtor_addr), dummy);
		free(dummy);
	}
};

template<class T>
void create_edit(Entity* parent, void* pdata, cComponentDealer* d, VariableInfo* v)
{
	auto e_edit = create_standard_edit(50.f, app.font_atlas_pixel, 1.f);
	parent->add_child(e_edit);
	((cText*)e_edit->find_component(cH("Text")))->set_text(to_wstring(*(T*)pdata));
	struct Capture
	{
		cComponentDealer* d;
		VariableInfo* v;
	}capture;
	capture.d = d;
	capture.v = v;
	((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
		auto& capture = *(Capture*)c;
		*(T*)((char*)capture.d->dummy + capture.v->offset()) = text[0] ? sto<T>(text) : 0;
		cmf(p2f<MF_v_vp_u>(capture.d->data_changed_addr), capture.d->dummy, capture.d->component, capture.v->name_hash());
	}, new_mail(&capture));
}

template<uint N, class T>
void create_vec_edit(Entity* parent, void* pdata, cComponentDealer* d, VariableInfo* v)
{
	for (auto i = 0; i < N; i++)
	{
		auto e_edit = create_standard_edit(50.f, app.font_atlas_pixel, 1.f);
		parent->add_child(wrap_standard_text(e_edit, false, app.font_atlas_pixel, 1.f, s2w(Vec<N, T>::coord_name(i))));
		((cText*)e_edit->find_component(cH("Text")))->set_text(to_wstring((*(Vec<N, T>*)pdata)[i]));
		struct Capture
		{
			cComponentDealer* d;
			VariableInfo* v;
			int i;
		}capture;
		capture.d = d;
		capture.v = v;
		capture.i = i;
		((cEdit*)e_edit->find_component(cH("Edit")))->add_changed_listener([](void* c, const wchar_t* text) {
			auto& capture = *(Capture*)c;
			(*(Vec<N, T>*)((char*)capture.d->dummy + capture.v->offset()))[capture.i] = text[0] ? sto<T>(text) : 0;
			cmf(p2f<MF_v_vp_u>(capture.d->data_changed_addr), capture.d->dummy, capture.d->component, capture.v->name_hash());
		}, new_mail(&capture));
	}
}

struct cInspectorPrivate : cInspector
{
	void* module;

	~cInspectorPrivate()
	{
		free_module(module);

		editor->inspector = nullptr;
	}

	void on_selected_changed()
	{
		looper().add_delay_event([](void* c) {
			auto thiz = *(cInspectorPrivate**)c;
			auto editor = thiz->editor;
			auto& dbs = editor->dbs();
			auto selected = editor->selected;

			auto e_layout = thiz->e_layout;
			e_layout->remove_all_children();
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
					((cText*)e_edit->find_component(cH("Text")))->set_text(s2w(selected->name()));
				}
				{
					auto e_item = create_item(L"visible");
					e_layout->add_child(e_item);

					auto e_checkbox = create_standard_checkbox(app.font_atlas_pixel, 1.f, L"");
					e_item->child(1)->add_child(e_checkbox);
				}

				for (auto i = 0; i < selected->component_count(); i++)
				{
					auto component = selected->component(i);

					auto e_component = Entity::create();
					e_layout->add_child(e_component);
					{
						auto c_element = cElement::create();
						c_element->inner_padding = Vec4f(4.f);
						c_element->background_frame_thickness = 2.f;
						c_element->background_frame_color = Vec4f(0, 0, 0, 255);
						e_component->add_component(c_element);

						auto c_aligner = cAligner::create();
						c_aligner->width_policy = SizeFitParent;
						e_component->add_component(c_aligner);

						auto c_layout = cLayout::create();
						c_layout->type = LayoutVertical;
						c_layout->item_padding = 2.f;
						e_component->add_component(c_layout);
					}

					auto udt = find_udt(dbs, H((std::string("c") + component->type_name).c_str()));

					auto c_dealer = new_component<cComponentDealer>();
					c_dealer->component = component;
					c_dealer->dummy = malloc(udt->size());
					{
						auto f = udt->find_function("ctor");
						if (f && f->parameter_count() == 0)
							cmf(p2f<MF_v_v>((char*)thiz->module + (uint)f->rva()), c_dealer->dummy);
					}
					c_dealer->dtor_addr = nullptr;
					{
						auto f = udt->find_function("dtor");
						if (f)
							c_dealer->dtor_addr = (char*)thiz->module + (uint)f->rva();
					}
					{
						auto f = udt->find_function("save");
						assert(f && f->return_type()->equal(TypeTagVariable, cH("void")) && f->parameter_count() == 1 && f->parameter_type(0)->equal(TypeTagPointer, cH("Component")));
						cmf(p2f<MF_v_vp>((char*)thiz->module + (uint)f->rva()), c_dealer->dummy, component);
					}
					{
						auto f = udt->find_function("data_changed");
						assert(f && f->return_type()->equal(TypeTagVariable, cH("void")) && f->parameter_count() == 2 && f->parameter_type(0)->equal(TypeTagPointer, cH("Component")) && f->parameter_type(1)->equal(TypeTagVariable, cH("uint")));
						c_dealer->data_changed_addr = (char*)thiz->module + (uint)f->rva();
					}
					e_component->add_component(c_dealer);

					auto e_name = Entity::create();
					e_component->add_child(e_name);
					{
						auto c_element = cElement::create();
						c_element->inner_padding = Vec4f(0.f, 0.f, 4.f + app.font_atlas_pixel->pixel_height, 0.f);
						e_name->add_component(c_element);

						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->color = Vec4c(30, 40, 160, 255);
						c_text->set_text(s2w(component->type_name));
						e_name->add_component(c_text);

						e_name->add_component(cLayout::create());
					}

					auto e_close = Entity::create();
					e_name->add_child(e_close);
					{
						e_close->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->set_text(Icon_WINDOW_CLOSE);
						e_close->add_component(c_text);

						auto c_aligner = cAligner::create();
						c_aligner->x_align = AlignxRight;
						e_close->add_component(c_aligner);
					}

					for (auto j = 0; j < udt->variable_count(); j++)
					{
						auto v = udt->variable(j);
						auto t = v->type();
						auto hash = t->hash();
						auto pdata = (char*)c_dealer->dummy + v->offset();

						auto e_item = create_item(s2w(v->name()));
						e_component->add_child(e_item);
						auto e_data = e_item->child(1);
						switch (t->tag())
						{
						case TypeTagEnumSingle:
						{
							auto info = find_enum(dbs, hash);
							create_enum_combobox(info, 120.f, app.font_atlas_pixel, 1.f, e_data);
							int idx;
							info->find_item(*(int*)pdata, &idx);
							auto combobox = (cCombobox*)e_data->child(0)->find_component(cH("Combobox"));
							combobox->set_index(idx, false);
							struct Capture
							{
								cComponentDealer* d;
								VariableInfo* v;
								EnumInfo* info;
							}capture;
							capture.d = c_dealer;
							capture.v = v;
							capture.info = info;
							combobox->add_changed_listener([](void* c, int idx) {
								auto& capture = *(Capture*)c;
								*(int*)((char*)capture.d->dummy + capture.v->offset()) = capture.info->item(idx)->value();
								cmf(p2f<MF_v_vp_u>(capture.d->data_changed_addr), capture.d->dummy, capture.d->component, capture.v->name_hash());
							}, new_mail(&capture));
						}
							break;
						case TypeTagEnumMulti:
						{
							auto info = find_enum(dbs, hash);
							create_enum_checkboxs(info, app.font_atlas_pixel, 1.f, e_data);
							for (auto k = 0; k < info->item_count(); k++)
							{
								auto vl = info->item(k)->value();
								auto checkbox = (cCheckbox*)e_data->child(k)->find_component(cH("Checkbox"));
								checkbox->set_checked(*(int*)pdata & vl, false);
								struct Capture
								{
									cComponentDealer* d;
									VariableInfo* v;
									int vl;
								}capture;
								capture.d = c_dealer;
								capture.v = v;
								capture.vl = vl;
								checkbox->add_changed_listener([](void* c, bool checked) {
									auto& capture = *(Capture*)c;
									auto pv = (int*)((char*)capture.d->dummy + capture.v->offset());
									if (checked)
										(*pv) |= capture.vl;
									else
										(*pv) &= ~capture.vl;
									cmf(p2f<MF_v_vp_u>(capture.d->data_changed_addr), capture.d->dummy, capture.d->component, capture.v->name_hash());
								}, new_mail(&capture));
							}
						}
							break;
						case TypeTagVariable:
							switch (hash)
							{
							case cH("bool"):
							{
								auto e_checkbox = create_standard_checkbox(app.font_atlas_pixel, 1.f, L"");
								e_data->add_child(e_checkbox);
								auto checkbox = (cCheckbox*)e_checkbox->find_component(cH("Checkbox"));
								checkbox->set_checked(*(bool*)pdata, false);
								struct Capture
								{
									cComponentDealer* d;
									VariableInfo* v;
								}capture;
								capture.d = c_dealer;
								capture.v = v;
								checkbox->add_changed_listener([](void* c, bool checked) {
									auto& capture = *(Capture*)c;
									*(bool*)((char*)capture.d->dummy + capture.v->offset()) = checked;
									cmf(p2f<MF_v_vp_u>(capture.d->data_changed_addr), capture.d->dummy, capture.d->component, capture.v->name_hash());
								}, new_mail(&capture));
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
								e_data->add_child(create_standard_edit(50.f, app.font_atlas_pixel, 1.f));
								((cText*)e_data->child(0)->find_component(cH("Text")))->set_text(s2w(*(std::string*)pdata));
								break;
							case cH("std::basic_string(wchar_t)"):
								e_data->add_child(create_standard_edit(50.f, app.font_atlas_pixel, 1.f));
								((cText*)e_data->child(0)->find_component(cH("Text")))->set_text(*(std::wstring*)pdata);
								break;
							}
							break;
						}
					}
				}
			}
		}, new_mail_p(this));
	}
};

void cInspector::on_selected_changed()
{
	((cInspectorPrivate*)this)->on_selected_changed();
}

void cInspector::update()
{
}

void open_inspector(cSceneEditor* editor, const Vec2f& pos)
{
	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = 200.f;
		c_element->height = 900.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker);

	auto tab = create_standard_docker_tab(app.font_atlas_pixel, L"Inspector", app.root);
	e_docker->child(0)->add_child(tab);

	auto e_page = get_docker_page_model()->copy();
	{
		((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(4.f);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_inspector = new_component<cInspectorPrivate>();
	e_page->add_component(c_inspector);
	c_inspector->module = load_module(L"flame_universe.dll");
	c_inspector->tab = (cDockerTab*)tab->find_component(cH("DockerTab"));
	c_inspector->editor = editor;
	editor->inspector = c_inspector;

	auto e_layout = Entity::create();
	{
		auto c_element = cElement::create();
		c_element->clip_children = true;
		e_layout->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		c_aligner->height_policy = SizeFitParent;
		e_layout->add_component(c_aligner);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_layout->add_component(c_layout);
	}

	c_inspector->e_layout = e_layout;

	e_page->add_child(wrap_standard_scrollbar(e_layout, ScrollbarVertical, true, 1.f));

	c_inspector->on_selected_changed();
}
