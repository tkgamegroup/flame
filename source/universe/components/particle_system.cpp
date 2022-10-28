#include "../../graphics/material.h"
#include "particle_system_private.h"
#include "node_private.h"
#include "camera_private.h"
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
			case PassForward:
				if ((draw_data.categories & CateParticle) && enable)
				{
					auto& mat = node->transform;
					auto rot = mat3(eulerAngleYXZ(radians(emitt_rotation.x), radians(emitt_rotation.y), radians(emitt_rotation.z)));

					acc_num += emitt_num * delta_time;
					auto n = int(acc_num);
					acc_num -= n;
					for (auto i = 0; i < n; i++)
					{
						auto& pt = particles.emplace_back();
						pt.pos = vec3(0.f);
						pt.ext = particle_ext;
						switch (emitt_type)
						{
						case "sphere"_h:
							pt.vel = sphericalRand(particle_speed);
							break;
						case "pie"_h:
						{
							auto rad = radians(linearRand(-emitt_angle, +emitt_angle));
							pt.vel = rot * vec3(cos(rad), 0.f, sin(rad)) * particle_speed;
						}
							break;
						}
						pt.col = particle_col;
						pt.time_max = particle_life_time;
						pt.time = pt.time_max;
						pt.rnd = linearRand(0.f, 1.f);
					}

					for (auto& pt : particles)
					{
						pt.pos += pt.vel * delta_time;
						pt.time -= delta_time;
					}

					auto& d = draw_data.particles.emplace_back();
					d.mat_id = material_res_id;
					d.pts.resize(particles.size());
					auto& camera_rot = sRenderer::instance()->camera->node->g_rot;
					auto i = 0;
					for (auto& src : particles)
					{
						auto& dst = d.pts[i];
						dst.pos = mat * vec4(src.pos, 1.f);
						dst.x_ext = +camera_rot[0] * src.ext.x;
						dst.y_ext = -camera_rot[1] * src.ext.y;
						dst.uv = vec4(vec2(0.f), vec2(1.f));
						dst.col = src.col;
						dst.time = src.time_max - src.time;
						i++;
					}

					for (auto it = particles.begin(); it != particles.end(); )
					{
						if (it->time <= 0.f)
							it = particles.erase(it);
						else
							it++;
					}
				}
				break;
			}
		}, "mesh"_h);
		node->measurers.add([this](AABB* ret) {
			*ret = AABB(AABB(vec3(0.f), particle_speed * particle_life_time + max(particle_ext.x, particle_ext.y)).get_points(node->transform));
			return true;
		}, "mesh"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
				dirty = true;
		}, "mesh"_h);
		data_listeners.add([this](uint hash) {
			if (hash == "enable"_h)
			{
				if (!enable)
					particles.clear();
				dirty = true;
			}
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