#include "../entity_private.h"
#include "../world_private.h"
#include "imgui_private.h"

namespace flame
{
	void cImguiPrivate::on_draw(void(*draw)(Capture& c), const Capture& capture)
	{
		callback.reset(new Closure(draw, capture));
	}

	void cImguiPrivate::on_entered_world()
	{
		auto world = entity->world;
		if (!world->first_imgui)
			world->first_imgui = entity;

	}

	void cImguiPrivate::on_left_world()
	{
		auto world = entity->world;
		if (world->first_imgui == entity)
			world->first_imgui = nullptr;
	}

	cImgui* cImgui::create(void* parms)
	{
		return new cImguiPrivate();
	}
}
