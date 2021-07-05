#pragma once

#include "particle_emitter.h"
#include "node_private.h"

namespace flame
{
	struct cParticleEmitterPrivate : cParticleEmitter, NodeDrawer
	{
		struct Particle
		{
			vec3 coord;
			float xlen;
			float ylen;
			vec3 rot;
			cvec4 col;

			vec3 mov_sp;
			vec3 rot_sp;
			int ttl;
		};

		std::filesystem::path img_src;
		std::string tile_name;

		cNodePrivate* node = nullptr;

		sRenderer* s_renderer = nullptr;

		int res_id = -1;
		int tile_id = -1;
		vec4 tile_uv;
		graphics::ImageView* iv = nullptr;
		graphics::ImageAtlas* atlas = nullptr;

		std::list<Particle> ptcs;

		float emt_prob = 0.05f;
		uint emt_num_min = 3;
		uint emt_num_max = 6;
		float emt_xlen_min = 0.1;
		float emt_xlen_max = 0.1;
		float emt_ylen_min = 0.1;
		float emt_ylen_max = 0.1;
		vec3 emt_rot_min = vec3(0.f);
		vec3 emt_rot_max = vec3(360.f);
		vec3 emt_mov_ori_min = vec3(0.f);
		vec3 emt_mov_ori_max = vec3(360.f);
		float emt_mov_sp_min = 0.02f;
		float emt_mov_sp_max = 0.02f;
		vec3 emt_rot_sp_min = vec3(0.f);
		vec3 emt_rot_sp_max = vec3(360.f);
		vec4 emt_hsva_min = vec4(0, 0.8, 0.8f, 0.5f);
		vec4 emt_hsva_max = vec4(360.f, 1.f, 1.f, 1.f);
		uint emt_ttl_min = 120;
		uint emt_ttl_max = 120;
		uint ptc_num_max = 0xffff;

		const wchar_t* get_img() const override { return img_src.c_str(); }
		void set_img(const std::filesystem::path& src);
		void set_img(const wchar_t* src) override { set_img(std::filesystem::path(src)); }
		const char* get_tile() const override { return tile_name.c_str(); }
		void set_tile(const std::string& name);
		void set_tile(const char* name) override { set_tile(std::string(name)); }

		void apply_src();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		void draw(sRendererPtr s_renderer) override;
	};
}
