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
			if (s_renderer->camera != this)
			{
				if (s_renderer->camera)
					s_renderer->camera->current = false;
				s_renderer->camera = this;
			}
		}
		else
		{
			if (s_renderer->camera == this)
				s_renderer->camera = nullptr;
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
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		apply_current();
	}

	void cCameraPrivate::on_left_world()
	{
		current = false;
		apply_current();
		s_renderer = nullptr;
	}

	void cCameraPrivate::set_current(bool v)
	{
		if (current == v)
			return;
		current = v;

		if (s_renderer)
			apply_current();
	}

	void cCameraPrivate::update_view()
	{
		node->update_transform();

		if (view_mark != node->transform_updated_times)
		{
			view_mark = node->transform_updated_times;

			view_inv = mat4(node->rot);
			view_inv[3] = vec4(node->g_pos, 1.f);
			view = inverse(view_inv);
		}
	}

	cCamera* cCamera::create(void* parms)
	{
		return f_new<cCameraPrivate>();
	}
}
