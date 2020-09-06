#include <flame/serialize.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "camera_private.h"
#include "mesh_instance_private.h"

namespace flame
{
	void cMeshInstancePrivate::set_model_index(int id)
	{
		if (model_index == id)
			return;
		model_index = id;
		if (node)
			node->mark_transform_dirty();
		if (!src.empty())
			src = "";
	}

	void cMeshInstancePrivate::set_mesh_index(int id)
	{
		if (mesh_index == id)
			return;
		mesh_index = id;
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshInstancePrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshInstancePrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cMeshInstancePrivate::apply_src()
	{
		model_index = -1;
		if (canvas && !src.empty())
			model_index = canvas->find_model(src.c_str());
	}

	void cMeshInstancePrivate::draw(graphics::Canvas* canvas, cCamera* _camera)
	{
		if (model_index != -1 && mesh_index != -1)
		{
			auto camera = (cCameraPrivate*)_camera;
			canvas->draw_mesh(model_index, mesh_index, camera->project_matrix, camera->view_matrix, node->transform, Mat4f(node->axes));
		}
	}

	cMeshInstance* cMeshInstance::create()
	{
		return f_new<cMeshInstancePrivate>();
	}
}
