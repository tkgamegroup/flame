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

		graphics::Queue::get()->wait_idle();
		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (!material_name.empty() && !material_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(material_name));
			if (material)
				graphics::Material::release(material);
		}
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

	void cParticleSystemPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data, cCameraPtr camera) {
			if (material_res_id == -1 || !enable)
				return;

			switch (draw_data.pass)
			{
			case PassForward:
				if ((draw_data.categories & CateParticle))
				{
					auto& mat = node->transform;
					auto& d = draw_data.particles.emplace_back();
					d.mat_id = material_res_id;
					d.ptcs.resize(particles.size());
					auto camera_rot = mat3(camera->view_mat_inv);
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
						if (texture_tiles != uvec2(1))
						{
							auto n = texture_tiles.x * texture_tiles.y;
							int i = texture_tiles_range.x + (dst.time / src.time_max) * ((texture_tiles_range.y == -1 ? n : texture_tiles_range.y) - texture_tiles_range.x);
							auto x = i % texture_tiles.x;
							auto y = i / texture_tiles.y;
							dst.uv = vec4(float(x) / texture_tiles.x, float(y) / texture_tiles.y, float(x + 1) / texture_tiles.x, float(y + 1) / texture_tiles.y);
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
		node->measurers.add([this](AABB& b) {
			b.expand(AABB(AABB(vec3(0.f), particle_speed * particle_life_time + max(particle_size.x, particle_size.y)).get_points(node->transform)));
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
				if (auto off = linearRand(emitt_offset_start, emitt_offset_end); off > 0.f)
					pt.pos += normalize(pt.vel) * off;
				break;
			case "Pie"_h:
				pt.vel = particle_speed > 0.f ? emitt_rotation_mat * dir_xz(linearRand(emitt_angle_start, emitt_angle_end)) * particle_speed : vec3(0.f);
				if (auto off = linearRand(emitt_offset_start, emitt_offset_end); off != 0.f)
					pt.pos += normalize(pt.vel) * off;
				if (auto off = linearRand(emitt_bitangent_offset_start, emitt_bitangent_offset_end); off != 0.f)
					pt.pos += emitt_rotation_mat[2] * off;
				break;
			}
			pt.rot = radians(linearRand(particle_rotation_start, particle_rotation_end));
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
