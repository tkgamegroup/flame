#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "camera_private.h"

namespace flame
{
	void cCameraPrivate::update_matrix()
	{
		if (project_dirty)
		{
			project_matrix;
			auto size = Vec2f(canvas->get_target(0)->get_image()->get_size());
			project_matrix = get_project_matrix(fovy * ANG_RAD, size.x() / size.y(), near, far);
		}
		if (view_dirty)
		{
			node->update_transform();
			view_matrix = get_view_matrix(Vec3f(node->transform[3]), 
				Vec3f(node->transform[3]) + node->rotate_matrix[2], node->rotate_matrix[1]);
		}
		if (project_dirty || view_dirty)
			vp_matrix = project_matrix * view_matrix;
		view_dirty = false;
		project_dirty = false;
	}

	cCamera* cCamera::create()
	{
		return f_new<cCameraPrivate>();
	}
}
