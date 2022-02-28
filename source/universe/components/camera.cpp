#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "camera_private.h"

namespace flame
{
	std::vector<cCameraPtr> cameras;

	void cCameraPrivate::update()
	{
		auto node = entity->get_component_i<cNodeT>(0);

		view_mat_inv = mat4(node->g_rot);
		view_mat_inv[3] = vec4(node->g_pos, 1.f);
		view_mat = inverse(view_mat_inv);

		proj_mat = perspective(radians(fovy), aspect, zNear, zFar);
		proj_mat[1][1] *= -1.f;
		proj_mat_inv = inverse(proj_mat);

		proj_view_mat = proj_mat * view_mat;
		proj_view_mat_inv = inverse(proj_view_mat);
		frustum = Frustum(proj_view_mat_inv);
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
