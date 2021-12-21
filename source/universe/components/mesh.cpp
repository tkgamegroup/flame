#include "../../graphics/device.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "armature_private.h"
#include "camera_private.h"
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

	void cMeshPrivate::set_skin(uint _skin)
	{
		if (skin == _skin)
			return;
		skin = _skin;
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;
		data_changed(S<"cast_shadow"_h>);
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
			model = graphics::Model::get(fn);
		else
		{
			if (!fn.is_absolute())
				fn = entity->get_src(src_id).parent_path() / fn;
			fn.make_preferred();
			model = graphics::Model::get(fn.c_str());
		}
		assert(model);

		if (sub_index >= model->meshes.size())
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

		if (mesh->get_bone_ids())
			parmature = entity->parent->get_component_t<cArmaturePrivate>();
	}

	void cMeshPrivate::draw(sRendererPtr s_renderer, bool shadow_pass)
	{
		if (mesh_id != -1)
		{
			auto get_idx = [&]() {
				if (parmature)
					return parmature->armature_id;
				if (frame < frames)
				{
					transform_id = s_renderer->add_mesh_transform(node->transform, node->g_rot);
					frame = frames;
				}
				return transform_id;
			};
			auto idx = get_idx();
			if (shadow_pass)
			{
				if (cast_shadow)
					s_renderer->draw_mesh(idx, mesh_id, skin, ShadingShadow);
			}
			else
				s_renderer->draw_mesh(idx, mesh_id, skin, shading_flags);
		}
	}

	bool cMeshPrivate::measure(AABB* ret)
	{
		if (!mesh)
			return false;
		auto b = mesh->bounds;
		vec3 ps[8];
		b.get_points(ps);
		b.reset();
		auto& mat = parmature ? parmature->node->transform : node->transform;
		for (auto i = 0; i < 8; i++)
			b.expand(mat * vec4(ps[i], 1.f));
		*ret = b;
		return true;
	}

	void cMeshPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);

		node->mark_drawing_dirty();
	}

	void cMeshPrivate::on_removed()
	{
		node = nullptr;
	}

	void cMeshPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

		apply_src();
		node->mark_bounds_dirty(false);
	}

	void cMeshPrivate::on_left_world()
	{
		s_renderer = nullptr;
		mesh_id = -1;
		mesh = nullptr;
	}

	cMesh* cMesh::create()
	{
		return new cMeshPrivate();
	}
}
