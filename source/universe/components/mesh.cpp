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
	cMeshPrivate::~cMeshPrivate()
	{
		node->drawers.remove("mesh"_h);
		node->shadow_drawers.remove("mesh"_h);
		node->measurers.remove("mesh"_h);

		if (mesh_res_id)
			sRenderer::instance()->release_mesh_res(mesh_res_id);
		if (material_res_id)
			sRenderer::instance()->release_material_res(material_res_id);
		if (model)
			graphics::Model::release(model);
	}

	void cMeshPrivate::on_init()
	{
		node->drawers.add([this](sRendererPtr renderer) {
			draw(renderer, false);
		}, "mesh"_h);
		node->shadow_drawers.add([this](sRendererPtr renderer) {
			draw(renderer, true);
		}, "mesh"_h);

		node->measurers.add([this](AABB* ret) {
			if (!mesh)
				return false;
			*ret = AABB(mesh->bounds.get_points(parmature ? parmature->node->transform : node->transform));
			return true;
		}, "mesh"_h);

		node->mark_transform_dirty();
	}

	void cMeshPrivate::set_model_name(const std::filesystem::path& _model_name)
	{
		if (model_name == _model_name)
			return;
		model_name = _model_name;

		graphics::ModelPtr _model = nullptr;
		graphics::MeshPtr _mesh = nullptr;
		graphics::MaterialPtr _material = nullptr;
		_model = graphics::Model::get(model_name);
		if (_model && !_model->meshes.empty())
		{
			_mesh = &model->meshes[0];
			if (!_mesh->materials.empty())
				_material = _mesh->materials[0];
		}
		if (model != _model)
		{
			if (model)
				graphics::Model::release(model);
			model = _model;
		}
		if (mesh != _mesh)
		{
			if (mesh_res_id != -1)
				sRenderer::instance()->release_mesh_res(mesh_res_id);
			mesh = _mesh;
			mesh_res_id = mesh ? sRenderer::instance()->get_mesh_res(mesh) : -1;
		}
		if (material != _material)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			material = _material;
			material_res_id = material ? sRenderer::instance()->get_material_res(material) : -1;
		}

		node->mark_transform_dirty();

		data_changed("model_name"_h);
		if (mesh_index != 0)
		{
			mesh_index = 0;
			data_changed("mesh_index"_h);
		}
		if (skin_index != 0)
		{
			skin_index = 0;
			data_changed("skin_index"_h);
		}
	}

	void cMeshPrivate::set_mesh_index(uint idx)
	{
		if (!model || idx >= model->meshes.size() || mesh_index == idx)
			return;
		mesh_index = idx;

		graphics::MaterialPtr _material = nullptr;
		auto _mesh = &model->meshes[mesh_index];
		if (!_mesh->materials.empty())
			_material = _mesh->materials[0];
		if (mesh != _mesh)
		{
			if (mesh_res_id != -1)
				sRenderer::instance()->release_mesh_res(mesh_res_id);
			mesh = _mesh;
			mesh_res_id = mesh ? sRenderer::instance()->get_mesh_res(mesh) : -1;
		}
		if (material != _material)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			material = _material;
			material_res_id = material ? sRenderer::instance()->get_material_res(material) : -1;
		}

		node->mark_transform_dirty();

		data_changed("mesh_index"_h);
		if (skin_index != 0)
		{
			skin_index = 0;
			data_changed("skin_index"_h);
		}
	}

	void cMeshPrivate::set_skin_index(uint idx)
	{
		if (!mesh || skin_index >= mesh->materials.size() || skin_index == idx)
			return;
		skin_index = idx;

		auto _material = mesh->materials[skin_index];
		if (material != _material)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			material = _material;
			material_res_id = material ? sRenderer::instance()->get_material_res(material) : -1;
		}

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

	void cMeshPrivate::draw(sRendererPtr renderer, bool shadow_pass)
	{
		if (mesh_res_id == -1 || material_res_id == -1 || instance_id == -1)
			return;
		if (!parmature && frame < (int)frames)
		{
			renderer->set_mesh_instance(instance_id, node->transform, node->g_rot);
			frame = frames;
		}

		if (shadow_pass)
		{
			if (cast_shadow)
				renderer->draw_mesh_occluder(instance_id, mesh_res_id, material_res_id);
		}
		else
			renderer->draw_mesh(instance_id, mesh_res_id, material_res_id);
	}

	void cMeshPrivate::on_active()
	{
		parmature = entity->get_parent_component_t<cArmatureT>();
		if (parmature)
			instance_id = parmature->instance_id;
		else
			instance_id = sRenderer::instance()->register_mesh_instance(-1);

		node->mark_transform_dirty();
	}

	void cMeshPrivate::on_inactive()
	{
		if (!parmature)
			sRenderer::instance()->register_mesh_instance(instance_id);
		instance_id = -1;
	}

	struct cMeshCreate : cMesh::Create
	{
		cMeshPtr operator()(EntityPtr e) override
		{
			return new cMeshPrivate();
		}
	}cMesh_create;
	cMesh::Create& cMesh::create = cMesh_create;
}
