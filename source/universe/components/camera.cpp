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

	void cCameraPrivate::get_points(float aspect, vec3* dst, float n, float f)
	{
		if (n < 0.f)
			n = near;
		if (f < 0.f)
			f = far;

		auto tan_hf_fovy = tan(radians(fovy * 0.5f));

		auto y1 = n * tan_hf_fovy;
		auto y2 = f * tan_hf_fovy;
		auto x1 = y1 * aspect;
		auto x2 = y2 * aspect;

		dst[0] = view_inv * vec4(-x1, y1, -n, 1.f);
		dst[1] = view_inv * vec4(x1, y1, -n, 1.f);
		dst[2] = view_inv * vec4(x1, -y1, -n, 1.f);
		dst[3] = view_inv * vec4(-x1, -y1, -n, 1.f);

		dst[4] = view_inv * vec4(-x2, y2, -f, 1.f);
		dst[5] = view_inv * vec4(x2, y2, -f, 1.f);
		dst[6] = view_inv * vec4(x2, -y2, -f, 1.f);
		dst[7] = view_inv * vec4(-x2, -y2, -f, 1.f);
	}

	void cCameraPrivate::get_planes(float aspect, Plane* dst, float n, float f)
	{
		vec3 ps[8];
		get_points(aspect, ps, n, f);
		dst[0] = Plane(ps[0], ps[1], ps[2]); // near
		dst[1] = Plane(ps[4], ps[7], ps[5]); // far
		dst[2] = Plane(ps[3], ps[7], ps[4]); // left
		dst[3] = Plane(ps[1], ps[5], ps[6]); // right
		dst[4] = Plane(ps[0], ps[4], ps[5]); // top
		dst[5] = Plane(ps[2], ps[6], ps[7]); // bottom
	}

	cCamera* cCamera::create(void* parms)
	{
		return f_new<cCameraPrivate>();
	}
}
