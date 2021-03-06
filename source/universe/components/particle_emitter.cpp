#include "../../graphics/device.h"
#include "../../graphics/image.h"
#include "../world_private.h"
#include "particle_emitter_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cParticleEmitterPrivate::set_img(const std::filesystem::path& src)
	{
		if (img_src == src)
			return;
		img_src = src;
		apply_src();
		if (entity)
			entity->component_data_changed(this, S<"img"_h>);
	}

	void cParticleEmitterPrivate::set_tile(const std::string& name)
	{
		if (tile_name == name)
			return;
		tile_name = name;
		apply_src();
		if (entity)
			entity->component_data_changed(this, S<"tile"_h>);
	}

	void cParticleEmitterPrivate::apply_src()
	{
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;

		if (!s_renderer || img_src.empty())
			return;
		auto device = graphics::Device::get_default();
		if (img_src.extension() != L".atlas")
		{
			auto img = graphics::Image::get(device, img_src.c_str(), false);
			fassert(img);
			iv = img->get_view();

			res_id = s_renderer->find_texture_res(iv);
			if (res_id == -1)
				res_id = s_renderer->set_texture_res(-1, iv, nullptr);
		}
		else
		{
			atlas = graphics::ImageAtlas::get(device, img_src.c_str());
			fassert(atlas);
			iv = atlas->get_image()->get_view();

			res_id = s_renderer->find_texture_res(iv);
			if (res_id == -1)
				res_id = s_renderer->set_texture_res(-1, iv, nullptr);

			if (!tile_name.empty())
			{
				graphics::ImageAtlas::TileInfo ti;
				tile_id = atlas->find_tile(tile_name.c_str(), &ti);
				fassert(tile_id != -1);
				tile_uv = ti.uv;
			}
		}

		if (node)
			node->mark_drawing_dirty();
	}

	void cParticleEmitterPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		node->mark_drawing_dirty();
	}

	void cParticleEmitterPrivate::on_removed()
	{
		node->mark_drawing_dirty();
	}

	void cParticleEmitterPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);
		apply_src();
	}

	void cParticleEmitterPrivate::on_left_world()
	{
		s_renderer = nullptr;
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;
	}

	mat3 euler_rot(const vec3& e)
	{
		return mat3(eulerAngleYXZ(radians(e.x), radians(e.y), radians(e.z)));
	}

	void cParticleEmitterPrivate::draw(sRendererPtr s_renderer, bool first, bool)
	{
		if (!first || res_id == -1 || (atlas && tile_id == -1))
			return;

		if (emt_tick % emt_intv == 0 && linearRand(0.f, 1.f) < emt_prob)
		{
			auto n = emt_num_rand > 0 ? linearRand(emt_num, emt_num + emt_num_rand) : emt_num;
			n = min(n, ptc_num_max - (uint)ptcs.size());
			for (auto i = 0; i < n; i++)
			{
				Particle p;
				p.pos = vec3(0.f);
				p.sz = any(greaterThan(emt_sz_rand, vec2(0.f))) ? linearRand(emt_sz, emt_sz + emt_sz_rand) : emt_sz;
				p.rot = any(greaterThan(emt_rot_rand, vec3(0.f))) ? linearRand(emt_rot, emt_rot + emt_rot_rand) : emt_rot;
				auto hsva = any(greaterThan(emt_hsva, vec4(0.f))) ? linearRand(emt_hsva, emt_hsva + emt_hsva_rand) : emt_hsva;
				p.col = vec4(rgbColor(hsva.rgb()), hsva.a);
				p.mov_sp = euler_rot(any(greaterThan(emt_mov_dir_rand, vec3(0.f))) ? 
					linearRand(emt_mov_dir, emt_mov_dir + emt_mov_dir_rand) : emt_mov_dir) *
					vec3(1.f, 0.f, 0.f) * (emt_mov_sp_rand > 0.f ? linearRand(emt_mov_sp, emt_mov_sp + emt_mov_sp_rand) : emt_mov_sp);
				p.rot_sp = any(greaterThan(emt_rot_sp_rand, vec3(0.f))) ? linearRand(emt_rot_sp, emt_rot_sp + emt_rot_sp_rand) : emt_rot_sp;
				p.ttl = emt_ttl_rand > 0 ? linearRand(emt_ttl, emt_ttl + emt_ttl_rand) : emt_ttl;
				ptcs.push_back(p);
			}
		}
		emt_tick++;

		for (auto& p : ptcs)
		{
			p.pos += p.mov_sp;
			p.rot += p.rot_sp;
			p.ttl--;
		}

		for (auto it = ptcs.begin(); it != ptcs.end();)
		{
			if (it->ttl <= 0)
				it = ptcs.erase(it);
			else
				it++;
		}

		node->update_transform();

		std::vector<graphics::Particle> renderer_ptcs;
		renderer_ptcs.resize(ptcs.size());
		auto idx = 0;
		for (auto& src : ptcs)
		{
			auto& dst = renderer_ptcs[idx];
			dst.pos = node->transform * vec4(src.pos, 1.f);
			auto rot = node->g_rot * euler_rot(src.rot);
			dst.xext = src.sz[0] * rot[0];
			dst.yext = src.sz[1] * rot[1];
			dst.uvs = vec4(0.f, 0.f, 1.f, 1.f);
			dst.col = src.col;
			idx++;
		}
		s_renderer->draw_particles(renderer_ptcs.size(), renderer_ptcs.data(), res_id);
	}

	cParticleEmitter* cParticleEmitter::create(void* parms)
	{
		return new cParticleEmitterPrivate();
	}
}
