#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
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

	void on_selected_changed()
	{
		looper().add_delay_event([](void* c) {
			auto thiz = *(cInspectorPrivate**)c;

			auto e_page = thiz->e_page;
			e_page->remove_all_children();
			auto selected = thiz->editor->selected;
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
				auto e_name = Entity::create();
				e_page->add_child(e_name);
				{
					e_name->add_component(cElement::create());

					auto c_text = cText::create(app.font_atlas_pixel);
					c_text->set_text(s2w(selected->name()) + L":");
					e_name->add_component(c_text);
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
		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
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
