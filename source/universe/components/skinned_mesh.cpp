#include "../../graphics/device.h"
#include "../../graphics/material.h"
#include "../../graphics/model.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "skinned_mesh_private.h"
#include "animator_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cSkinnedMeshPrivate::~cSkinnedMeshPrivate()
	{
		node->drawers.remove("mesh"_h);
		node->measurers.remove("mesh"_h);
		node->data_listeners.remove("mesh"_h);

		graphics::Queue::get()->wait_idle();
		if (mesh_res_id != -1)
			sRenderer::instance()->release_mesh_res(mesh_res_id);
		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (!mesh_name.empty())
			AssetManagemant::release(Path::get(mesh_name));
		if (!mesh_name.empty() && !mesh_name.native().starts_with(L"0x"))
		{
			if (mesh)
				graphics::Mesh::release(mesh);
		}
		if (!material_name.empty() && !material_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(material_name));
			if (material)
				graphics::Material::release(material);
		}
	}

	void cSkinnedMeshPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data, cCameraPtr camera) {
			if (mesh_res_id == -1 || instance_id == -1 || material_res_id == -1)
				return;

			switch (draw_data.pass)
			{
			case PassInstance:
				if (animator)
					animator->update_instance();
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
			if (mesh && animator)
			{
				auto pc = animator->node_to_cluster[node];
				if (!pc)
					b.expand(AABB(mesh->bounds.get_points(animator->node->transform)));
				else
					b.expand(AABB(mesh->bounds.get_points(pc->pose.m)));
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

	void cSkinnedMeshPrivate::set_mesh_name(const std::filesystem::path& name)
	{
		if (mesh_name == name)
			return;

		auto old_one = mesh;
		auto old_raw = !mesh_name.empty() && mesh_name.native().starts_with(L"0x");
		if (!mesh_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(mesh_name));
		}
		mesh_name = name;
		mesh = nullptr;
		if (!mesh_name.empty())
		{
			if (!mesh_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(mesh_name));
				mesh = !mesh_name.empty() ? graphics::Mesh::get(mesh_name) : nullptr;
			}
			else
				mesh = (graphics::MeshPtr)s2u_hex<uint64>(mesh_name.string());
		}

		if (mesh != old_one)
		{
			if (mesh_res_id != -1)
				sRenderer::instance()->release_mesh_res(mesh_res_id);
			mesh_res_id = mesh ? sRenderer::instance()->get_mesh_res(mesh, -1) : -1;
		}
		if (!old_raw && old_one)
			graphics::Mesh::release(old_one);

		node->mark_drawing_dirty();
		data_changed("mesh_name"_h);
	}

	void cSkinnedMeshPrivate::set_material_name(const std::filesystem::path& name)
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

	void cSkinnedMeshPrivate::set_color(const cvec4& col)
	{
		if (color == col)
			return;
		color = col;

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("color"_h);
	}

	void cSkinnedMeshPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;
		node->mark_drawing_dirty();
		data_changed("cast_shadow"_h);
	}

	void cSkinnedMeshPrivate::set_enable_render(bool v)
	{
		if (enable_render == v)
			return;
		enable_render = v;
		node->mark_drawing_dirty();
		data_changed("enable_render"_h);
	}

	void cSkinnedMeshPrivate::on_active()
	{
		animator = entity->get_parent_component<cAnimatorT>();
		if (instance_id != 0)
		{
			if (animator)
				instance_id = animator->instance_id;
		}

		node->mark_transform_dirty();
	}

	void cSkinnedMeshPrivate::on_inactive()
	{
		if (instance_id != 0)
			instance_id = -1;
	}

	struct cSkinnedMeshCreate : cSkinnedMesh::Create
	{
		cSkinnedMeshPtr operator()(EntityPtr e) override
		{
			return new cSkinnedMeshPrivate();
		}
	}cSkinnedMesh_create;
	cSkinnedMesh::Create& cSkinnedMesh::create = cSkinnedMesh_create;
}
