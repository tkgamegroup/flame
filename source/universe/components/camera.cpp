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
			project_matrix = make_perspective_project_matrix(fovy * ANG_RAD, size.x() / size.y(), near, far);
		}
		node->update_transform();
		if (view_dirty)
			view_matrix = inverse(Mat4f(Mat<3, 4, float>(node->axes, Vec3f(0.f)), Vec4f(node->pos, 1.f)));
		if (project_dirty || view_dirty)
			vp_matrix = project_matrix * view_matrix;
		project_dirty = false;
		view_dirty = false;
	}

	void cCameraPrivate::on_local_data_changed(Component* t, uint64 h)
	{
		if (t == node && h == S<ch("transform")>::v)
			view_dirty = true;
	}

	cCamera* cCamera::create()
	{
		return f_new<cCameraPrivate>();
	}
}
