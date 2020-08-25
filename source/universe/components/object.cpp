#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "camera_private.h"
#include "object_private.h"

namespace flame
{
	void cObjectPrivate::draw(graphics::Canvas* canvas, cCamera* _camera)
	{
		auto camera = (cCameraPrivate*)_camera;
		canvas->add_object(0, camera->vp_matrix * node->transform, transpose(inverse(camera->view_matrix * node->transform)));
	}

	cObject* cObject::create()
	{
		return f_new<cObjectPrivate>();
	}
}
