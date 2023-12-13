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
		texture_sheet_size = uvec2(1);
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
			if (material_res_id != -1)
			{
				auto& res = sRenderer::instance()->get_material_res_info(material_res_id);
				if (res.mat->color_map != - 1)
				{
					auto& tex = res.texs[res.mat->color_map];
					graphics::ImageConfig cfg;
					graphics::Image::get_config(tex.second->filename, &cfg);
					texture_sheet_size = cfg.sheet_size;
				}
			}
		}
		if (!old_raw && old_one)
			graphics::Material::release(old_one);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
	}

	void cParticleSystemPrivate::set_trail_material_name(const std::filesystem::path& name)
	{
		if (trail_material_name == name)
			return;

		auto old_one = trail_material;
		auto old_raw = !trail_material_name.empty() && trail_material_name.native().starts_with(L"0x");
		if (!trail_material_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(trail_material_name));
		}
		trail_material_name = name;
		trail_material = nullptr;
		trail_texture_sheet_size = uvec2(1);
		if (!trail_material_name.empty())
		{
			if (!trail_material_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(trail_material_name));
				trail_material = !trail_material_name.empty() ? graphics::Material::get(trail_material_name) : nullptr;
			}
			else
				trail_material = (graphics::MaterialPtr)s2u_hex<uint64>(trail_material_name.string());
		}

		if (trail_material != old_one)
		{
			if (trail_material_res_id != -1)
				sRenderer::instance()->release_material_res(trail_material_res_id);
			trail_material_res_id = trail_material ? sRenderer::instance()->get_material_res(trail_material, -1) : -1;
			if (trail_material_res_id != -1)
			{
				auto& res = sRenderer::instance()->get_material_res_info(trail_material_res_id);
				if (res.mat->color_map != -1)
				{
					auto& tex = res.texs[res.mat->color_map];
					graphics::ImageConfig cfg;
					graphics::Image::get_config(tex.second->filename, &cfg);
					trail_texture_sheet_size = cfg.sheet_size;
				}
			}
		}
		if (!old_raw && old_one)
			graphics::Material::release(old_one);

		node->mark_drawing_dirty();
		data_changed("trail_material_name"_h);
	}

	static float interpolate(const std::vector<vec2>& ctrl_pts, float t, float curvedness)
	{
		if (ctrl_pts.size() == 1)
			return ctrl_pts[0].y;
		auto it = std::upper_bound(ctrl_pts.begin(), ctrl_pts.end(), t,
			[](float v, const auto& i) { return v < i.x; });
		if (it == ctrl_pts.begin())
			return it->y;
		if (it == ctrl_pts.end())
			return ctrl_pts.back().y;

		auto p0 = (it - 1) == ctrl_pts.begin() ? (2.f * ctrl_pts[0] - ctrl_pts[1]) : *(it - 2);
		auto p1 = *(it - 1);
		auto p2 = *it;
		auto p3 = (it + 1) == ctrl_pts.end() ? (2.f * ctrl_pts.rbegin()[0] - ctrl_pts.rbegin()[1]) : *(it + 1);
		return Curve<2>::interpolate(p0, p1, p2, p3, map_01(t, p1.x, p2.x), curvedness).y;
	}

	static void render_particle(const Particle& src, ParticleDrawData::Ptc& dst, ParticleRenderType render_type, const mat4& mat, const mat3& camera_rot, const uvec2& texture_sheet_size)
	{
		dst.time = src.lifetime - src.time;
		dst.pos = mat * vec4(src.pos, 1.f);
		switch (render_type)
		{
		case ParticleBillboard:
			if (src.ang != 0.f)
			{
				auto rot = mat3(rotate(mat4(1.f), src.ang, camera_rot[2]));
				dst.x_ext = rot * +camera_rot[0] * src.size.x;
				dst.y_ext = rot * -camera_rot[1] * src.size.y;
			}
			else
			{
				dst.x_ext = +camera_rot[0] * src.size.x;
				dst.y_ext = -camera_rot[1] * src.size.y;
			}
			break;
		case ParticleHorizontalBillboard:
			if (src.ang != 0.f)
			{
				auto rot = mat3(rotate(mat4(1.f), src.ang, vec3(0.f, 1.f, 0.f)));
				dst.x_ext = rot[0] * src.size.x;
				dst.y_ext = rot[2] * src.size.y;
			}
			else
			{
				dst.x_ext = +vec3(1.f, 0.f, 0.f) * src.size.x;
				dst.y_ext = +vec3(0.f, 0.f, 1.f) * src.size.y;
			}
			break;
		case ParticleVerticalBillboard:
			if (src.ang != 0.f)
			{
				auto rot = mat3(rotate(mat4(1.f), src.ang, vec3(0.f, 0.f, 1.f)));
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
		if (texture_sheet_size != uvec2(1))
		{
			auto n = texture_sheet_size.x * texture_sheet_size.y;
			int i = (dst.time / src.lifetime) * n;
			auto x = i % texture_sheet_size.x;
			auto y = i / texture_sheet_size.y;
			dst.uv = vec4(float(x) / texture_sheet_size.x, float(y) / texture_sheet_size.y, float(x + 1) / texture_sheet_size.x, float(y + 1) / texture_sheet_size.y);
		}
		else
			dst.uv = vec4(vec2(0.f), vec2(1.f));
		dst.col = src.col;
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
					auto& d_pt = draw_data.particles.emplace_back();
					d_pt.mat_id = material_res_id;
					d_pt.ptcs.resize(particles.size());
					auto camera_rot = mat3(camera->view_mat_inv);
					auto i_pt = 0;
					for (auto& src : particles)
					{
						auto& dst = d_pt.ptcs[i_pt];
						render_particle(src, dst, render_type, mat, camera_rot, texture_sheet_size);
						i_pt++;
					}
					if (enable_trail && trail_material_res_id != -1)
					{
						auto& d_tr = draw_data.particles.emplace_back();
						d_tr.mat_id = trail_material_res_id;
						for (auto& tr : trails)
						{
							auto i_pt = d_tr.ptcs.size();
							d_tr.ptcs.resize(d_tr.ptcs.size() + tr.second.size());
							for (auto& src : tr.second)
							{
								auto& dst = d_tr.ptcs[i_pt];
								render_particle(src, dst, trail_render_type, mat, camera_rot, trail_texture_sheet_size);
								i_pt++;
							}
						}
					}
				}
				break;
			}
		}, "mesh"_h);
		node->measurers.add([this](AABB& b) {
			auto max_travel = particle_speed_max * particle_life_time_max;
			AABB bounds(vec3(0.f), max_travel + max(particle_size.x, particle_size.y));
			b.expand(AABB(bounds.get_points(node->transform)));
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
		else
			emitt_timer = 0.f;
		if (emitt_duration == 0.f || emitt_timer < emitt_duration)
			accumulated_num += emitt_num * delta_time;
		auto num = int(accumulated_num);
		accumulated_num -= num;
		for (auto i = 0; i < num; i++)
		{
			auto& pt = particles.emplace_back();
			pt.pos = vec3(0.f);
			pt.size = particle_size;
			pt.vel = vec3(0.f);
			switch (emitt_type)
			{
			case ParticleEmittSphere:
				if (auto sp = linearRand(particle_speed_min, particle_speed_max); sp > 0.f)
					pt.vel = sphericalRand(sp);
				if (auto off = linearRand(emitt_offset_min, emitt_offset_max); off > 0.f)
					pt.pos += normalize(pt.vel) * off;
				break;
			case ParticleEmittPie:
				if (auto sp = linearRand(particle_speed_min, particle_speed_max); sp > 0.f)
					pt.vel = emitt_rotation_mat * dir_xz(linearRand(emitt_angle_min, emitt_angle_max)) * sp;
				if (auto off = linearRand(emitt_offset_min, emitt_offset_max); off != 0.f)
					pt.pos += normalize(pt.vel) * off;
				if (auto off = linearRand(emitt_bitangent_offset_min, emitt_bitangent_offset_max); off != 0.f)
					pt.pos += emitt_rotation_mat[2] * off;
				break;
			case ParticleEmittCone:
				break;
			}
			pt.ang = radians(linearRand(particle_angle_min, particle_angle_max));
			pt.col = particle_color;
			pt.lifetime = linearRand(particle_life_time_min, particle_life_time_max);
			pt.time = pt.lifetime;
			pt.id = next_particle_id++;
		}

		for (auto it = particles.begin(); it != particles.end(); )
		{
			if (it->time <= 0.f)
				it = particles.erase(it);
			else
			{
				it->pos += it->vel * delta_time;
				it->time -= delta_time;
				auto t = (it->lifetime - it->time) / it->lifetime;
				if (particle_scale_over_lifetime)
				{
					auto& ctrl_pts = particle_scale_curve.ctrl_points;
					if (!ctrl_pts.empty())
						it->size = particle_size * interpolate(ctrl_pts, t, particle_scale_curve.curvedness);
				}
				if (particle_alpha_over_lifetime)
				{
					auto& ctrl_pts = particle_alpha_curve.ctrl_points;
					if (!ctrl_pts.empty())
						it->col.a = clamp(interpolate(ctrl_pts, t, particle_alpha_curve.curvedness), 0.f, 1.f) * 255.f;
				}
				if (particle_brightness_over_lifetime)
				{
					auto& ctrl_pts = particle_brightness_curve.ctrl_points;
					if (!ctrl_pts.empty())
						it->col.rgb = vec3(particle_color.rgb()) * clamp(interpolate(ctrl_pts, t, particle_brightness_curve.curvedness), 0.f, 1.f);
				}
				it++;
			}
		}

		if (enable_trail > 0.f)
			trail_emitt_timer += delta_time;
		else
			trail_emitt_timer = 0.f;
		if (trail_emitt_timer > trail_emitt_tick)
		{
			trail_emitt_timer = 0.f;
			for (auto& pt : particles)
			{
				Particle vt;
				vt.pos = pt.pos;
				vt.lifetime = linearRand(trail_life_time_min, trail_life_time_max);
				vt.time = vt.lifetime;
				trails[pt.id].push_back(vt);
			}
		}

		for (auto& t : trails)
		{
			for (auto it = t.second.begin(); it != t.second.end(); it++)
			{
				if (it->time <= 0.f)
					it = t.second.erase(it);
				else
				{
					// TODO: move the trail vertex according to gravity (maybe)
					it->time -= delta_time;
					it++;
				}
			}
		}

		for (auto it = trails.begin(); it != trails.end(); )
		{
			if (it->second.empty())
				it = trails.erase(it);
			else
				it++;
		}
	}

	void cParticleSystemPrivate::reset()
	{
		particles.clear();
		emitt_timer = 0.f;
		accumulated_num = 0.f;
		trail_emitt_timer = 0.f;
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

	std::vector<Particle> cParticleSystemPrivate::get_trail(uint id)
	{
		std::vector<Particle> ret;
		if (auto it = trails.find(id); it != trails.end())
		{
			for (auto& pt : it->second)
				ret.push_back(pt);
		}
		return ret;
	}

	void cParticleSystemPrivate::set_trail(uint id, const std::vector<Particle>& vts)
	{
		if (auto it = trails.find(id); it != trails.end())
		{
			auto& list = it->second;
			for (auto& v : vts)
				list.push_back(v);
		}
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
