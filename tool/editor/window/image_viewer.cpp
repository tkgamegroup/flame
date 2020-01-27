#include <flame/graphics/image.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

#include "../app.h"
#include "image_viewer.h"

void open_image_viewer(uint id, const Vec2f& pos)
{
	auto image_size = (Vec2f)app.s_2d_renderer->canvas->get_image(id)->image()->size;

	ui::push_parent(app.root);
	ui::next_element_pos = pos;
	ui::next_element_size = image_size * 0.5f + 20.f;
	ui::e_begin_docker_floating_container();
	ui::e_begin_docker();
	ui::e_begin_docker_page(L"Image Viewer");
	ui::current_entity()->get_component(cElement)->inner_padding_ = Vec4f(8.f);

	ui::e_image(id << 16, image_size, 8.f)->get_component(cElement)->scale_ = 0.5f;

	ui::e_end_docker_page();
	ui::e_end_docker();
	ui::e_end_docker_floating_container();
	ui::pop_parent();
}
