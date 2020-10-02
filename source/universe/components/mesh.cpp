#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "node_private.h"
#include "mesh_private.h"

namespace flame
{
	void cMeshPrivate::set_model_id(int id)
	{
		if (model_id == id)
			return;
		model_id = id;
		if (node)
			node->mark_transform_dirty();
		if (!src.empty())
			src = "";
	}

	void cMeshPrivate::set_mesh_id(int id)
	{
		if (mesh_id == id)
			return;
		mesh_id = id;
		if (node)
			node->mark_transform_dirty();
		if (!src.empty())
			src = "";
	}

	void cMeshPrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	void cMeshPrivate::apply_src()
	{
		model_id = -1;
		mesh_id = -1;
		if (canvas && !src.empty())
		{
			auto sp = SUS::split(src, '.');
			if (sp.size() == 2)
			{
				model_id = canvas->find_resource(graphics::ResourceModel, sp[0].c_str());
				if (model_id != -1)
				{
					auto model = (graphics::Model*)canvas->get_resource(graphics::ResourceModel, model_id);
					mesh_id = model->find_mesh(sp[1].c_str());
				}
			}
		}
	}

	void cMeshPrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cMeshPrivate::draw(graphics::Canvas* canvas)
	{
		if (model_id != -1 && mesh_id != -1)
			canvas->draw_mesh(model_id, mesh_id, node->transform, Mat4f(node->axes), cast_shadow);
	}

	cMesh* cMesh::create()
	{
		return f_new<cMeshPrivate>();
	}
}
