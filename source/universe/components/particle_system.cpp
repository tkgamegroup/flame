#include "../../graphics/image.h"
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
			AssetManagemant::release(Path::get(material_name));
	}

	void cParticleSystemPrivate::set_emitt_rotation(const vec3& r)
	{
		if (emitt_rotation == r)
			return;
		emitt_rotation = r;
		emitt_rotation_mat = mat3(eulerAngleYXZ(radians(emitt_rotation.x), radians(emitt_rotation.y), radians(emitt_rotation.z)));

		data_changed("emitt_rotation"_h);
	}

	void cParticleSystemPrivate::set_material_name(const std::filesystem::path& name)
	{
		if (material_name == name)
			return;
		if (!material_name.empty())
			AssetManagemant::release(Path::get(material_name));
		material_name = name;
		if (!material_name.empty())
			AssetManagemant::get(Path::get(material_name));

		auto _material = !material_name.empty() ? graphics::Material::get(material_name) : nullptr;
		if (material != _material)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			if (material)
				graphics::Material::release(material);
			material = _material;
			material_res_id = material ? sRenderer::instance()->get_material_res(material, -1) : -1;

			if (material_res_id != -1)
			{
				if (material->color_map != -1)
				{
					auto& res = sRenderer::instance()->get_material_res_info(material_res_id);
					color_map_tiles = res.texs[material->color_map].second->tiles;
				}
				else
					color_map_tiles = uvec2(1);
			}
		}
		else if (_material)
			graphics::Material::release(_material);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
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
					auto& d = draw_data.particles.emplace_back();
					d.mat_id = material_res_id;
					d.ptcs.resize(particles.size());
					auto& camera_rot = sRenderer::instance()->camera->node->g_rot;
					auto i = 0;
					for (auto& src : particles)
					{
						auto& dst = d.ptcs[i];
						dst.time = src.time_max - src.time;
						dst.pos = mat * vec4(src.pos, 1.f);
						switch (render_type)
						{
						case "Billboard"_h:
							if (src.rot != 0.f)
							{
								auto rot = mat3(rotate(mat4(1.f), src.rot, camera_rot[2]));
								dst.x_ext = rot * +camera_rot[0] * src.size.x;
								dst.y_ext = rot * -camera_rot[1] * src.size.y;
							}
							else
							{
								dst.x_ext = +camera_rot[0] * src.size.x;
								dst.y_ext = -camera_rot[1] * src.size.y;
							}
							break;
						case "HorizontalBillboard"_h:
							if (src.rot != 0.f)
							{
								auto rot = mat3(rotate(mat4(1.f), src.rot, vec3(0.f, 1.f, 0.f)));
								dst.x_ext = rot[0] * src.size.x;
								dst.y_ext = rot[2] * src.size.y;
							}
							else
							{
								dst.x_ext = +vec3(1.f, 0.f, 0.f) * src.size.x;
								dst.y_ext = +vec3(0.f, 0.f, 1.f) * src.size.y;
							}
							break;
						case "VerticalBillboard"_h:
							if (src.rot != 0.f)
							{
								auto rot = mat3(rotate(mat4(1.f), src.rot, vec3(0.f, 0.f, 1.f)));
								dst.x_ext = rot[0] * src.size.x;
								dst.y_ext = rot[1] * src.size.y;
							}
							else
							{
								dst.x_ext = +vec3(1.f, 0.f, 0.f) * src.size.x;
								dst.y_ext = +vec3(0.f, 1.f, 0.f) * src.size.y;
							}
							break;
						}
						if (color_map_tiles != uvec2(1))
						{
							int n = (dst.time / src.time_max) * (color_map_tiles.x * color_map_tiles.y);
							auto x = n % color_map_tiles.x;
							auto y = n / color_map_tiles.y;
							dst.uv = vec4(float(x) / color_map_tiles.x, float(y) / color_map_tiles.y, float(x + 1) / color_map_tiles.x, float(y + 1) / color_map_tiles.y);
						}
						else
							dst.uv = vec4(vec2(0.f), vec2(1.f));
						dst.col = src.col;
						i++;
					}
				}
				break;
			}
		}, "mesh"_h);
		node->measurers.add([this](AABB* ret) {
			*ret = AABB(AABB(vec3(0.f), particle_speed * particle_life_time + max(particle_size.x, particle_size.y)).get_points(node->transform));
			return true;
		}, "mesh"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
				dirty = true;
		}, "mesh"_h);

		node->mark_transform_dirty();
	}

	void cParticleSystemPrivate::on_inactive()
	{
		particles.clear();
		emitt_timer = 0.f;
	}

	void cParticleSystemPrivate::start()
	{
		accumulated_num = emitt_start_num;
	}

	void cParticleSystemPrivate::update()
	{
		if (emitt_duration > 0.f)
			emitt_timer += delta_time;
		if (emitt_duration == 0.f || emitt_timer < emitt_duration)
			accumulated_num += emitt_num * delta_time;
		auto num = int(accumulated_num);
		accumulated_num -= num;
		for (auto i = 0; i < num; i++)
		{
			auto& pt = particles.emplace_back();
			pt.pos = vec3(0.f);
			pt.size = particle_size;
			switch (emitt_type)
			{
			case "Sphere"_h:
				pt.vel = particle_speed > 0.f ? sphericalRand(particle_speed) : vec3(0.f);
				break;
			case "Pie"_h:
				pt.vel = particle_speed > 0.f ? emitt_rotation_mat * dir_xz(linearRand(-emitt_angle, +emitt_angle)) * particle_speed : vec3(0.f);
				break;
			}
			pt.rot = linearRand(0.f, 360.f);
			pt.col = particle_col;
			pt.time_max = particle_life_time;
			if (particle_life_time_start > 0.f)
			{
				pt.time = pt.time_max - particle_life_time_start;
				pt.pos = pt.vel * particle_life_time_start;
			}
			else
				pt.time = pt.time_max;
			pt.rnd = linearRand(0.f, 1.f);
		}

		for (auto& pt : particles)
		{
			pt.pos += pt.vel * delta_time;
			pt.time -= delta_time;
		}

		for (auto it = particles.begin(); it != particles.end(); )
		{
			if (it->time <= 0.f)
				it = particles.erase(it);
			else
				it++;
		}
	}

	std::vector<Particle> cParticleSystemPrivate::get_particles()
	{
		std::vector<Particle> ret;
		for (auto& pt : particles)
			ret.push_back(pt);
		return ret;
	}

	void cParticleSystemPrivate::set_particles(const std::vector<Particle>& pts)
	{
		particles.clear();
		for (auto& pt : pts)
			particles.push_back(pt);
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
