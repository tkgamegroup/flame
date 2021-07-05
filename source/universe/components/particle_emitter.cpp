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

	void cParticleEmitterPrivate::draw(sRendererPtr s_renderer)
	{
		if (res_id == -1)
			return;
		if (atlas && tile_id == -1)
			return;

		if (linearRand(0.f, 1.f) < emt_prob)
		{
			auto n = linearRand(emt_num_min, emt_num_max);
			n = min(n, ptc_num_max - (uint)ptcs.size());
			for (auto i = 0; i < n; i++)
			{
				Particle p;
				p.coord = vec3(0.f);
				p.xlen = linearRand(emt_xlen_min, emt_xlen_max);
				p.ylen = linearRand(emt_ylen_min, emt_ylen_max);
				p.rot = linearRand(emt_rot_min, emt_rot_max);
				auto hsva = linearRand(emt_hsva_min, emt_hsva_max);
				p.col = cvec4(vec4(rgbColor(hsva.rgb()), hsva.a) * 255.f);
				p.mov_sp = euler_rot(linearRand(emt_mov_ori_min, emt_mov_ori_max)) * 
					vec3(1.f, 0.f, 0.f) * linearRand(emt_mov_sp_min, emt_mov_sp_max);
				p.rot_sp = linearRand(emt_rot_sp_min, emt_rot_sp_max);
				p.ttl = linearRand(emt_ttl_min, emt_ttl_max);
				ptcs.push_back(p);
			}
		}

		for (auto& p : ptcs)
		{
			p.coord += p.mov_sp;
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

		std::vector<sRenderer::Particle> renderer_ptcs;
		renderer_ptcs.resize(ptcs.size());
		auto idx = 0;
		for (auto& src : ptcs)
		{
			auto& dst = renderer_ptcs[idx];
			dst.coord = node->transform * vec4(src.coord, 1.f);
			auto rot = node->g_rot * euler_rot(src.rot);
			dst.xext = src.xlen * rot[0];
			dst.yext = src.ylen * rot[1];
			dst.uvs = vec4(0.f, 0.f, 1.f, 1.f);
			dst.color = src.col;
			idx++;
		}
		s_renderer->draw_particles(renderer_ptcs.size(), renderer_ptcs.data(), res_id);
	}

	cParticleEmitter* cParticleEmitter::create(void* parms)
	{
		return new cParticleEmitterPrivate();
	}
}
