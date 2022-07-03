#include "../../graphics/device.h"
#include "../../graphics/material.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "mesh_private.h"
#include "armature_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	std::pair<std::filesystem::path, uint> parse_name(const std::filesystem::path& src)
	{
		auto sp = SUW::split(src.wstring(), '#');
		auto name = sp.empty() ? L"" : sp.front();
		if (!name.starts_with(L"standard_"))
			name = Path::get(name);
		auto idx = 0;
		if (sp.size() > 1)
		{
			SUW::strip_head_if(sp[1], L"mesh");
			idx = s2t<uint>(sp[1]);
		}
		return std::make_pair(name, idx);
	}

	cMeshPrivate::~cMeshPrivate()
	{
		node->drawers.remove("mesh"_h);
		node->measurers.remove("mesh"_h);

		if (mesh_res_id != -1)
			sRenderer::instance()->release_mesh_res(mesh_res_id);
		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (auto model_and_index = parse_name(mesh_name); !model_and_index.first.empty())
			AssetManagemant::release_asset(Path::get(model_and_index.first));
		if (model)
			graphics::Model::release(model);
	}

	void cMeshPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (mesh_res_id == -1 || instance_id == -1 || material_res_id == -1)
				return;

			switch (draw_data.pass)
			{
			case "instance"_h:
				if (!parmature)
					sRenderer::instance()->set_mesh_instance(instance_id, node->transform, node->g_rot);
				break;
			case "draw"_h:
				if (draw_data.category == "mesh"_h)
					draw_data.meshes.emplace_back(instance_id, mesh_res_id, material_res_id);
				break;
			case "occulder"_h:
				if (cast_shadow)
				{
					if (draw_data.category == "mesh"_h)
						draw_data.meshes.emplace_back(instance_id, mesh_res_id, material_res_id);
				}
				break;
			}
		}, "mesh"_h);
		node->measurers.add([this](AABB* ret) {
			if (!mesh)
				return false;
			*ret = AABB(mesh->bounds.get_points(parmature ? parmature->node->transform : node->transform));
			return true;
		}, "mesh"_h);

		node->mark_transform_dirty();
	}

	void cMeshPrivate::set_mesh_name(const std::filesystem::path& name)
	{
		if (mesh_name == name)
			return;

		auto model_and_index = parse_name(mesh_name);
		auto _model_and_index = parse_name(name);
		mesh_name = name;

		if (model_and_index.first != _model_and_index.first)
		{
			if (!model_and_index.first.empty())
				AssetManagemant::release_asset(Path::get(model_and_index.first));
			if (!_model_and_index.first.empty())
				AssetManagemant::get_asset(Path::get(_model_and_index.first));
		}

		graphics::ModelPtr _model = nullptr;
		graphics::MeshPtr _mesh = nullptr;
		if (!_model_and_index.first.empty())
			_model = graphics::Model::get(_model_and_index.first);
		if (_model && !_model->meshes.empty())
		{
			if (_model_and_index.second < _model->meshes.size())
				_mesh = &_model->meshes[_model_and_index.second];
		}
		if (model != _model)
		{
			if (model)
				graphics::Model::release(model);
			model = _model;
		}
		else if (_model)
			graphics::Model::release(_model);
		if (mesh != _mesh)
		{
			if (mesh_res_id != -1)
				sRenderer::instance()->release_mesh_res(mesh_res_id);
			mesh = _mesh;
			mesh_res_id = mesh ? sRenderer::instance()->get_mesh_res(mesh, -1) : -1;
		}

		node->mark_transform_dirty();
		data_changed("mesh_name"_h);
	}

	void cMeshPrivate::set_material_name(const std::filesystem::path& name)
	{
		if (material_name == name)
			return;
		material_name = name;

		auto _material = !material_name.empty() ? graphics::Material::get(material_name) : nullptr;
		if (material != _material)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			if (material)
				graphics::Material::release(material);
			material = _material;
			material_res_id = material ? sRenderer::instance()->get_material_res(material, -1) : -1;
		}
		else if (_material)
			graphics::Material::release(_material);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;
		data_changed("cast_shadow"_h);
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
