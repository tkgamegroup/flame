#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/window.h>

#include "../app.h"
#include "image_viewer.h"

void open_image_viewer(uint id, const Vec2f& pos)
{
	auto image_size = app.canvas->get_image(id)->image()->size;

	auto e_container = get_docker_container_model()->copy();
	app.root->add_child(e_container);
	{
		auto c_element = (cElement*)e_container->find_component(cH("Element"));
		c_element->x = pos.x();
		c_element->y = pos.y();
		c_element->width = image_size.x() + 20.f;
		c_element->height = image_size.y() + 20.f;
	}

	auto e_docker = get_docker_model()->copy();
	e_container->add_child(e_docker);
	e_docker->child(0)->add_child(create_standard_docker_tab(app.font_atlas_pixel, L"Image Viewer", app.root));

	auto e_page = get_docker_page_model()->copy();
	e_docker->child(1)->add_child(e_page);
	{
		((cElement*)e_page->find_component(cH("Element")))->inner_padding = Vec4f(8.f);

		auto c_layout = cLayout::create();
		c_layout->width_fit_children = false;
		c_layout->height_fit_children = false;
		e_page->add_component(c_layout);
	}

	auto e_image = Entity::create();
	e_page->add_child(e_image);
	{
		auto c_element = cElement::create();
		c_element->width = image_size.x();
		c_element->height = image_size.y();
		e_image->add_component(c_element);

		auto c_image = cImage::create();
		c_image->id = id;
		e_image->add_component(c_image);
	}
}
