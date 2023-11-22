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
		auto s = src.wstring();
		auto sp = SUW::split(s, '#');
		std::wstring name = sp.empty() ? L"" : std::wstring(sp.front());
		if (!name.starts_with(L"standard_"))
			name = Path::get(name);
		auto idx = 0;
		if (sp.size() > 1)
		{
			auto mesh_name = std::wstring(sp[1]);
			SUW::strip_head_if(mesh_name, L"mesh");
			idx = s2t<uint>(mesh_name);
		}
		return std::make_pair(name, idx);
	}

	cMeshPrivate::~cMeshPrivate()
	{
		node->drawers.remove("mesh"_h);
		node->measurers.remove("mesh"_h);
		node->data_listeners.remove("mesh"_h);

		graphics::Queue::get()->wait_idle();
		if (mesh_res_id != -1)
			sRenderer::instance()->release_mesh_res(mesh_res_id);
		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (auto model_and_index = parse_name(mesh_name); !model_and_index.first.empty())
			AssetManagemant::release(Path::get(model_and_index.first));
		if (!mesh_name.empty() && !mesh_name.native().starts_with(L"0x"))
		{
			if (mesh && mesh->model)
				graphics::Model::release(mesh->model);
		}
		if (!material_name.empty() && !material_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(material_name));
			if (material)
				graphics::Material::release(material);
		}
	}

	void cMeshPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data, cCameraPtr camera) {
			if (mesh_res_id == -1 || instance_id == -1 || material_res_id == -1)
				return;

			switch (draw_data.pass)
			{
			case PassInstance:
				if (parmature)
					parmature->update_instance();
				else
				{
					if (dirty)
					{
						sRenderer::instance()->set_mesh_instance(instance_id, node->transform, transpose(inverse(mat3(node->transform))), color);
						dirty = false;
					}
				}
				break;
			case PassGBuffer:
				if (enable_render && (draw_data.categories & CateMesh) && (material->render_queue == graphics::RenderQueue::Opaque || material->render_queue == graphics::RenderQueue::AlphaTest))
					draw_data.meshes.emplace_back(instance_id, mesh_res_id, material_res_id);
				break;
			case PassForward:
				if (enable_render && (draw_data.categories & CateMesh) && material->render_queue == graphics::RenderQueue::Transparent)
					draw_data.meshes.emplace_back(instance_id, mesh_res_id, material_res_id);
				break;
			case PassOcculder:
				if (enable_render && (draw_data.categories & CateMesh) && cast_shadow && material->render_queue != graphics::RenderQueue::Transparent)
					draw_data.meshes.emplace_back(instance_id, mesh_res_id, material_res_id);
				break;
			case PassPickUp:
				if ((draw_data.categories & CateMesh))
					draw_data.meshes.emplace_back(instance_id, mesh_res_id, material_res_id);
				break;
			}
		}, "mesh"_h);
		node->measurers.add([this](AABB& b) {
			if (mesh)
			{
				if (!parmature)
					b.expand(AABB(mesh->bounds.get_points(node->transform)));
				else
				{
					auto pb = parmature->bone_node_map[node];
					if (!pb)
						b.expand(AABB(mesh->bounds.get_points(parmature->node->transform)));
					else
						b.expand(AABB(mesh->bounds.get_points(pb->pose.m)));
				}
			}
		}, "mesh"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
				dirty = true;
		}, "mesh"_h);
		data_listeners.add([this](uint hash) {
			if (hash == "enable"_h)
				dirty = true;
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
				AssetManagemant::release(Path::get(model_and_index.first));
			if (!_model_and_index.first.empty())
				AssetManagemant::get(Path::get(_model_and_index.first));
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
		if (!mesh || mesh->model != _model)
		{
			if (mesh && mesh->model)
				graphics::Model::release(mesh->model);
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

		auto old_one = material;
		auto old_raw = !material_name.empty() && material_name.native().starts_with(L"0x");
		if (!material_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(material_name));
		}
		material_name = name;
		material = nullptr;
		if (!material_name.empty())
		{
			if (!material_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(material_name));
				material = !material_name.empty() ? graphics::Material::get(material_name) : nullptr;
			}
			else
				material = (graphics::MaterialPtr)s2u_hex<uint64>(material_name.string());
		}

		if (material != old_one)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			material_res_id = material ? sRenderer::instance()->get_material_res(material, -1) : -1;
		}
		if (!old_raw && old_one)
			graphics::Material::release(old_one);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
	}

	void cMeshPrivate::set_color(const cvec4& col)
	{
		if (color == col)
			return;
		color = col;

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("color"_h);
	}

	void cMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;
		node->mark_drawing_dirty();
		data_changed("cast_shadow"_h);
	}

	void cMeshPrivate::set_enable_render(bool v) 
	{
		if (enable_render == v)
			return;
		enable_render = v;
		node->mark_drawing_dirty();
		data_changed("enable_render"_h);
	}

	void cMeshPrivate::on_active()
	{
		parmature = entity->get_parent_component<cArmatureT>();
		if (instance_id != 0)
		{
			if (parmature)
				instance_id = parmature->instance_id;
			else
				instance_id = sRenderer::instance()->register_mesh_instance(-1);
		}

		node->mark_transform_dirty();
	}

	void cMeshPrivate::on_inactive()
	{
		if (instance_id != 0)
		{
			if (!parmature)
				sRenderer::instance()->register_mesh_instance(instance_id);
			instance_id = -1;
		}
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
