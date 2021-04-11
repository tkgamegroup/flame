#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "camera_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cCameraPrivate::apply_current()
	{
		if (current)
		{
			if (renderer->camera != this)
			{
				if (renderer->camera)
					renderer->camera->current = false;
				renderer->camera = this;
			}
		}
		else
		{
			if (renderer->camera == this);
			renderer->camera = nullptr;
		}
	}

	void cCameraPrivate::on_added()
	{
		node = entity->get_component_t<cNodePrivate>();
		fassert(node);

		drawer = node->add_drawer([](Capture& c, sRenderer* render) {
			auto thiz = c.thiz<cCameraPrivate>();
			thiz->draw(render);
		}, Capture().set_thiz(this));
		node->mark_drawing_dirty();
	}

	void cCameraPrivate::on_removed()
	{
		node->remove_drawer(drawer);
		node = nullptr;
	}

	void cCameraPrivate::on_entered_world()
	{
		renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(renderer);

		apply_current();
	}

	void cCameraPrivate::on_left_world()
	{
		current = false;
		apply_current();
		renderer = nullptr;
	}

	void cCameraPrivate::draw(sRenderer* render)
	{
		// TODO: fix below
		//auto vp = canvas->get_viewport();
		//auto size = max(vec2(1.f), vp.RB - vp.LT);
		//canvas->set_camera(fovy, size.x / size.y, near, far, node->g_rot, node->g_pos);
	}

	void cCameraPrivate::set_current(bool v)
	{
		if (current == v)
			return;
		current = v;

		if (renderer)
			apply_current();
	}

	cCamera* cCamera::create(void* parms)
	{
		return f_new<cCameraPrivate>();
	}
}
