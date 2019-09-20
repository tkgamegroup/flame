#include <flame/graphics/font.h>
#include <flame/foundation/serialize.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "inspector.h"
#include "scene_editor.h"

struct cInspectorPrivate : cInspector
{
	~cInspectorPrivate()
	{
		editor->inspector = nullptr;
	}

	Entity* create_data_item(const std::wstring& title)
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
			c_text->sdf_scale = 0.6f;
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

	void on_selected_changed()
	{
		looper().add_delay_event([](void* c) {
			auto thiz = *(cInspectorPrivate**)c;
			auto editor = thiz->editor;
			auto selected = editor->selected;

			auto e_page = thiz->e_page;
			e_page->remove_all_children();
			if (!selected)
			{
				auto e_text = Entity::create();
				e_page->add_child(e_text);
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
					auto e_item = thiz->create_data_item(L"name");
					e_page->add_child(e_item);

					auto e_edit = create_standard_edit(100.f, app.font_atlas_pixel, 1.f);
					e_item->child(1)->add_child(e_edit);
					((cText*)e_edit->find_component(cH("Text")))->set_text(s2w(selected->name()));
				}
				{
					auto e_item = thiz->create_data_item(L"visible");
					e_page->add_child(e_item);

					auto e_checkbox = create_standard_checkbox(app.font_atlas_pixel, 1.f, L"");
					e_item->child(1)->add_child(e_checkbox);
				}

				for (auto i = 0; i < selected->component_count(); i++)
				{
					auto c = selected->component(i);

					auto e_c = Entity::create();
					e_page->add_child(e_c);
					{
						auto c_element = cElement::create();
						c_element->inner_padding = Vec4f(4.f);
						c_element->background_frame_thickness = 2.f;
						c_element->background_frame_color = Vec4f(0, 0, 0, 255);
						e_c->add_component(c_element);

						auto c_layout = cLayout::create();
						c_layout->type = LayoutVertical;
						c_layout->item_padding = 2.f;
						e_c->add_component(c_layout);
					}

					auto e_name = Entity::create();
					e_c->add_child(e_name);
					{
						e_name->add_component(cElement::create());

						auto c_layout = cLayout::create();
						c_layout->type = LayoutHorizontal;
						c_layout->item_padding = 2.f;
						e_name->add_component(c_layout);
					}

					auto e_text = Entity::create();
					e_name->add_child(e_text);
					{
						e_text->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->set_text(s2w(c->type_name));
						e_text->add_component(c_text);
					}

					auto e_close = Entity::create();
					e_name->add_child(e_close);
					{
						e_close->add_component(cElement::create());

						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->set_text(Icon_WINDOW_CLOSE);
						e_close->add_component(c_text);
					}

					auto udt = find_udt(editor->dbs(), H((std::string("c") + c->type_name).c_str()));
					for (auto j = 0; j < udt->variable_count(); j++)
					{
						auto v = udt->variable(j);
						auto t = v->type();

						auto e_item = thiz->create_data_item(s2w(v->name()));
						e_c->add_child(e_item);
						auto e_data = e_item->child(1);
						switch (t->tag())
						{
						case TypeTagEnumSingle:
							break;
						case TypeTagEnumMulti:
							break;
						case TypeTagVariable:
							switch (t->hash())
							{

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
		c_layout->item_padding = 4.f;
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}
	e_docker->child(1)->add_child(e_page);

	auto c_inspector = new_component<cInspectorPrivate>();
	e_page->add_component(c_inspector);
	c_inspector->tab = (cDockerTab*)tab->find_component(cH("DockerTab"));
	c_inspector->editor = editor;
	c_inspector->e_page = e_page;
	editor->inspector = c_inspector;

	c_inspector->on_selected_changed();
}
