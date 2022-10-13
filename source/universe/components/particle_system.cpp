#include "../../graphics/material.h"
#include "particle_system_private.h"
#include "node_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cParticleSystemPrivate::~cParticleSystemPrivate()
	{
		node->drawers.remove("particle_system"_h);
		node->measurers.remove("particle_system"_h);
		node->data_listeners.remove("particle_system"_h);

		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (!material_name.empty())
			AssetManagemant::release_asset(Path::get(material_name));
	}

	void cParticleSystemPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (material_res_id == -1)
				return;

			switch (draw_data.pass)
			{
			case "forward"_h:
				if (draw_data.category == "particle"_h && enable)
				{
					auto& d = draw_data.particles.emplace_back();
					d.mat_id = material_res_id;

				}
				break;
			}
			}, "mesh"_h);
		node->measurers.add([this](AABB* ret) {
			*ret = AABB(AABB(vec3(0.f), 10.f).get_points(node->transform));
			return true;
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

	void cParticleSystemPrivate::set_material_name(const std::filesystem::path& name)
	{
		if (material_name == name)
			return;
		if (!material_name.empty())
			AssetManagemant::release_asset(Path::get(material_name));
		material_name = name;
		if (!material_name.empty())
			AssetManagemant::get_asset(Path::get(material_name));

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

	struct cParticleSystemCreate : cParticleSystem::Create
	{
		cParticleSystemPtr operator()(EntityPtr e) override
		{
			return new cParticleSystemPrivate();
		}
	}cParticleSystem_create;
	cParticleSystem::Create& cParticleSystem::create = cParticleSystem_create;
}
