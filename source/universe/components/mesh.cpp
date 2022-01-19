#include "../../graphics/device.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "armature_private.h"
#include "../systems/node_renderer_private.h"

namespace flame
{
	cMeshPrivate::~cMeshPrivate()
	{
		node->drawers.remove(drawer_lis);
		node->measurers.remove(measurer_lis);
	}

	void cMeshPrivate::on_init()
	{
		drawer_lis = node->drawers.add([this](sNodeRendererPtr renderer, bool shadow_pass) {
			draw(renderer, shadow_pass);
		});

		measurer_lis = node->measurers.add([this](AABB* ret) {
			if (!mesh)
				return false;
			AABB b;
			b.reset();
			vec3 ps[8];
			mesh->bounds.get_points(ps);
			auto& mat = /* parmature ? parmature->node->transform : */ node->transform;
			for (auto i = 0; i < 8; i++)
				b.expand(mat * vec4(ps[i], 1.f));
			*ret = b;
			return true;
		});

		node->mark_drawing_dirty();
	}

	void cMeshPrivate::set_model_name(const std::filesystem::path& _model_name)
	{
		if (model_name == _model_name)
			return;
		model_name = _model_name;
		apply_src();
		node->mark_transform_dirty();
		data_changed("model_name"_h);
	}

	void cMeshPrivate::set_mesh_index(uint idx)
	{
		if (mesh_index == idx)
			return;
		mesh_index = idx;
		apply_src();
		node->mark_transform_dirty();
		data_changed("mesh_index"_h);
	}

	void cMeshPrivate::set_skin_index(uint idx)
	{
		if (skin_index == idx)
			return;
		skin_index = idx;
		node->mark_drawing_dirty();
		data_changed("skin_index"_h);
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;
		data_changed("cast_shadow"_h);
	}

	void cMeshPrivate::set_shading_flags(ShadingFlags flags)
	{
		if (shading_flags == flags)
			return;
		shading_flags = flags;
		node->mark_drawing_dirty();
		data_changed("shading_flags"_h);
	}

	void cMeshPrivate::apply_src()
	{
		mesh = nullptr;
		mesh_id = -1;
		if (model_name.empty())
			return;

		graphics::Model* model = nullptr;
		auto fn = model_name;
		if (fn.extension().empty())
			model = graphics::Model::get(fn);
		else
		{
			fn = Path::get(fn);
			model = graphics::Model::get(fn);
		}
		if (!model)
			return;

		if (mesh_index >= model->meshes.size())
			return;
		mesh = &model->meshes[mesh_index];
	}

	void cMeshPrivate::draw(sNodeRendererPtr renderer, bool shadow_pass)
	{
		if (mesh_id == -1 && mesh)
		{
			mesh_id = renderer->find_mesh_res(mesh);
			if (mesh_id == -1)
			{
				mesh_id = renderer->set_mesh_res(-1, mesh);
				if (mesh_id == -1)
				{
					mesh = nullptr;
					return;
				}
			}
		}
		if (mesh_id == -1)
		{
			mesh = nullptr;
			return;
		}
		auto get_idx = [&]() {
			//if (parmature)
			//	return parmature->armature_id;
			if (frame < frames)
			{
				transform_id = renderer->add_mesh_transform(node->transform, node->g_rot);
				frame = frames;
			}
			return transform_id;
		};
		auto idx = get_idx();
		if (shadow_pass)
		{
			if (cast_shadow)
				renderer->draw_mesh(idx, mesh_id, skin_index, ShadingShadow);
		}
		else
			renderer->draw_mesh(idx, mesh_id, skin_index, shading_flags);
	}

	void cMeshPrivate::on_active()
	{
		parmature = entity->get_parent_component_t<cArmatureT>();

		apply_src();
		node->mark_transform_dirty();
	}

	void cMeshPrivate::on_inactive()
	{
		mesh = nullptr;
		mesh_id = -1;
	}

	struct cMeshCreatePrivate : cMesh::Create
	{
		cMeshPtr operator()(EntityPtr e) override
		{
			return new cMeshPrivate();
		}
	}cMesh_create_private;
	cMesh::Create& cMesh::create = cMesh_create_private;
}
