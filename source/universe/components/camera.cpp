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
		assert(node);

		node->mark_drawing_dirty();
	}

	void cCameraPrivate::on_removed()
	{
		node = nullptr;
	}

	void cCameraPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

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

	void cCameraPrivate::set_fovy(float v) 
	{
		if (fovy == v)
			return;
		fovy = v;
	}

	void cCameraPrivate::set_near(float v) 
	{
		if (near == v)
			return;
		near = v;
	}

	void cCameraPrivate::set_far(float v) 
	{
		if (far == v)
			return;
		far = v;
	}

	void cCameraPrivate::set_screen_size(const uvec2& sz)
	{
		if (screen_size == sz)
			return;
		screen_size = sz;
		aspect = (float)sz.x / (float)sz.y;
		proj_dirty = true;
	}

	void cCameraPrivate::update_view()
	{
		node->update_transform();

		if (view_mark == node->transform_updated_times)
			return;

		view_mark = node->transform_updated_times;

		view_inv = mat4(node->rot);
		view_inv[3] = vec4(node->g_pos, 1.f);
		view = inverse(view_inv);
	}

	void cCameraPrivate::update_proj()
	{
		if (!proj_dirty)
			return;

		proj = perspective(radians(fovy), aspect, near, far); // z range: 0 to 1
		proj[1][1] *= -1.f;
		proj_inv = inverse(proj);
	}

	void cCameraPrivate::get_points(vec3* dst, float n, float f)
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

	Frustum cCameraPrivate::get_frustum(float n, float f)
	{
		vec3 ps[8];
		get_points(ps, n, f);
		return Frustum(ps);
	}

	vec3 cCameraPrivate::screen_to_world(const uvec2& pos)
	{
		auto p = proj_inv * vec4((vec2)pos / (vec2)screen_size * 2.f - 1.f, near, 1.f);
		p = p / p.w;
		return view_inv * p;
	}

	ivec2 cCameraPrivate::world_to_screen(const vec3& pos, const ivec4& border)
	{
		auto p = proj * view * vec4(pos, 1.f);
		p = p / p.w;
		if (p.z > 0.f && p.z < 1.f)
		{
			auto ret = ivec2((p.xy() + 1.f) * 0.5f * (vec2)screen_size);
			if (ret.x > border.x &&
				ret.x < screen_size.x - border.z &&
				ret.y > border.y &&
				ret.y < screen_size.y - border.w)
			return ret;
		}
		return ivec2(10000);
	}

	cCamera* cCamera::create()
	{
		return new cCameraPrivate();
	}
}
