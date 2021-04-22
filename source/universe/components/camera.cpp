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
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		node->mark_drawing_dirty();
	}

	void cCameraPrivate::on_removed()
	{
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
