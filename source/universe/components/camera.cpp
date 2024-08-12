#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "camera_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	std::vector<cCameraPtr> cameras;

	vec2 cCameraPrivate::world_to_screen(const vec3& pos, vec3* out_clip_coord)
	{
		auto p = proj_view_mat * vec4(pos, 1.f);
		p /= p.w;
		if (out_clip_coord)
			*out_clip_coord = p;
		if (p.z < -1.f || p.z > 1.f)
			return vec2(-1.f);
		for (auto& render_target : sRenderer::instance()->render_targets)
		{
			if (render_target->camera == this)
			{
				auto ext = render_target->target_extent();
				auto target_ext = render_target->target_extent();
				if (target_ext.x <= 0.f || target_ext.y <= 0.f)
					return vec2(-1.f);
				return (p.xy() * 0.5f + 0.5f) * target_ext;
			}
		}
		return vec2(-1.f);
	}

	void cCameraPrivate::on_active()
	{
		for (auto it = cameras.begin(); it != cameras.end(); it++)
		{
			if (entity->compare_depth((*it)->entity))
			{
				cameras.insert(it, this);
				return;
			}
		}
		cameras.push_back(this);
	}

	void cCameraPrivate::on_inactive()
	{
		std::erase_if(cameras, [&](auto c) {
			return c == this;
		});
	}

	void cCameraPrivate::update_matrices()
	{
		if (node)
		{
			view_mat_inv = mat3(node->g_qut);
			view_mat_inv[3] = vec4(node->global_pos(), 1.f);
			view_mat = inverse(view_mat_inv);

			proj_mat = perspective(radians(fovy), aspect, zNear, zFar);
			proj_mat[1][1] *= -1.f;

			proj_mat_inv = inverse(proj_mat);

			proj_view_mat = proj_mat * view_mat;
			proj_view_mat_inv = inverse(proj_view_mat);
			frustum = Frustum(proj_view_mat_inv);
		}
	}

	struct cCameraCreate : cCamera::Create
	{
		cCameraPtr operator()(EntityPtr e) override
		{
			return new cCameraPrivate;
		}
	}cCamera_create;
	cCamera::Create& cCamera::create = cCamera_create;

	struct cCameraList : cCamera::List
	{
		const std::vector<cCameraPtr>& operator()() override
		{
			return cameras;
		}
	}cCamera_list;
	cCamera::List& cCamera::list = cCamera_list;
}
