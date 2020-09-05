#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "camera_private.h"
#include "object_private.h"

namespace flame
{
	void cObjectPrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cObjectPrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cObjectPrivate::apply_src()
	{
		model_idx = -1;
		if (canvas && !src.empty())
			model_idx = canvas->find_model(src.c_str());
		Entity::report_data_changed(this, S<ch("model_idx")>::v);
	}

	void cObjectPrivate::draw(graphics::Canvas* canvas, cCamera* _camera)
	{
		if (model_idx != -1)
		{
			auto camera = (cCameraPrivate*)_camera;
			canvas->add_object(model_idx, camera->project_matrix, camera->view_matrix, node->transform, Mat4f(node->axes));
		}
	}

	cObject* cObject::create()
	{
		return f_new<cObjectPrivate>();
	}
}
