#include <flame/serialize.h>
#include <flame/graphics/model.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "mesh_instance_private.h"

namespace flame
{
	void cMeshInstancePrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		get_mesh();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshInstancePrivate::set_mesh_index(int id)
	{
		if (mesh_index == id)
			return;
		mesh_index = id;
		get_mesh();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshInstancePrivate::set_cast_shadow(bool v)
	{
		cast_shadow = v;
	}

	void cMeshInstancePrivate::on_gain_canvas()
	{
		get_mesh();
	}

	void cMeshInstancePrivate::get_mesh()
	{
		mesh = nullptr;
		if (canvas && !src.empty())
		{
			model_index = canvas->find_resource(graphics::ResourceModel, src.c_str());
			if (model_index != -1)
			{
				mesh = ((graphics::Model*)canvas->get_resource(graphics::ResourceModel, model_index))->get_mesh(mesh_index);
				Entity::report_data_changed(this, S<ch("mesh")>::v);
			}
		}
	}

	void cMeshInstancePrivate::draw(graphics::Canvas* canvas)
	{
		if (model_index != -1 && mesh_index != -1)
			canvas->draw_mesh(model_index, mesh_index, node->transform, Mat4f(node->axes), cast_shadow);
	}

	cMeshInstance* cMeshInstance::create()
	{
		return f_new<cMeshInstancePrivate>();
	}
}
