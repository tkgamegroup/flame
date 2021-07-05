#include "../../graphics/device.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "armature_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cMeshPrivate::set_src(const std::filesystem::path& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::set_sub_index(uint idx)
	{
		if (sub_index == idx)
			return;
		sub_index = idx;
		apply_src();
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow != v)
		{
			cast_shadow = v;
			if (entity)
				entity->component_data_changed(this, S<"cast_shadow"_h>);
		}
	}

	void cMeshPrivate::set_shading_flags(ShadingFlags flags)
	{
		if (shading_flags == flags)
			return;
		shading_flags = flags;
		if (node)
			node->mark_transform_dirty();
	}

	void cMeshPrivate::apply_src()
	{
		mesh_id = -1;
		mesh = nullptr;
		if (!s_renderer || src.empty())
			return;

		graphics::Model* model = nullptr;
		auto fn = src;
		if (fn.extension().empty())
			model = graphics::Model::get_standard(fn.c_str());
		else
		{
			if (!fn.is_absolute())
				fn = entity->get_src(src_id).parent_path() / fn;
			fn.make_preferred();
			model = graphics::Model::get(fn.c_str());
		}
		fassert(model);

		if (sub_index >= model->get_meshes_count())
			return;
		mesh = model->get_mesh(sub_index);

		mesh_id = s_renderer->find_mesh_res(mesh);
		if (mesh_id == -1)
		{
			mesh_id = s_renderer->set_mesh_res(-1, mesh);
			if (mesh_id == -1)
			{
				mesh = nullptr;
				return;
			}
		}

		if (model->get_bones_count() > 0)
			pani = entity->parent->get_component_t<cArmaturePrivate>();
	}

	void cMeshPrivate::draw(sRendererPtr s_renderer)
	{
		if (mesh_id != -1)
			s_renderer->draw_mesh(node, mesh_id, cast_shadow, pani ? pani->armature_id : -1, shading_flags);
	}

	bool cMeshPrivate::measure(AABB* b)
	{
		if (!mesh)
			return false;
		*b = AABB(mesh->get_lower_bound(), mesh->get_upper_bound());
		return true;
	}

	void cMeshPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		node->mark_drawing_dirty();
	}

	void cMeshPrivate::on_removed()
	{
		node = nullptr;
	}

	void cMeshPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		apply_src();
	}

	void cMeshPrivate::on_left_world()
	{
		s_renderer = nullptr;
		mesh_id = -1;
		mesh = nullptr;
	}

	cMesh* cMesh::create(void* parms)
	{
		return new cMeshPrivate();
	}
}
