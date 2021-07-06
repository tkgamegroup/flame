#pragma once

#include "particle_emitter.h"
#include "node_private.h"

namespace flame
{
	struct cParticleEmitterPrivate : cParticleEmitter, NodeDrawer
	{
		struct Particle
		{
			vec3 pos;
			vec2 sz;
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

		uint emt_intv = 1;
		float emt_prob = 0.05f;
		uint emt_num_min = 3;
		uint emt_num_max = 6;
		vec2 emt_sz_min = vec2(0.1);
		vec2 emt_sz_max = vec2(0.1);
		vec3 emt_rot_min = vec3(0.f);
		vec3 emt_rot_max = vec3(0.f);
		vec3 emt_mov_dir_min = vec3(0.f);
		vec3 emt_mov_dir_max = vec3(360.f);
		float emt_mov_sp_min = 0.02f;
		float emt_mov_sp_max = 0.02f;
		vec3 emt_rot_sp_min = vec3(0.f);
		vec3 emt_rot_sp_max = vec3(0.f);
		vec4 emt_hsva_min = vec4(0.f, 0.f, 1.f, 1.f);
		vec4 emt_hsva_max = vec4(0.f, 0.f, 1.f, 1.f);
		uint emt_ttl_min = 120;
		uint emt_ttl_max = 120;
		uint ptc_num_max = 0xffff;

		uint emt_tick = 0;

		const wchar_t* get_img() const override { return img_src.c_str(); }
		void set_img(const std::filesystem::path& src);
		void set_img(const wchar_t* src) override { set_img(std::filesystem::path(src)); }
		const char* get_tile() const override { return tile_name.c_str(); }
		void set_tile(const std::string& name);
		void set_tile(const char* name) override { set_tile(std::string(name)); }

		uint get_emt_intv() const override { return emt_intv; }
		void set_emt_intv(uint v) { emt_intv = v; }
		float get_emt_prob() const override { return emt_prob; }
		void set_emt_prob(float v) override { emt_prob = v; }
		uint get_emt_num_min() const override { return emt_num_min; }
		void set_emt_num_min(uint v) override { emt_num_min = v; }
		uint get_emt_num_max() const override { return emt_num_max; }
		void set_emt_num_max(uint v) override { emt_num_max = v; }
		vec2 get_emt_sz_min() const override { return emt_sz_min; }
		void set_emt_sz_min(const vec2& v) override { emt_sz_min = v; }
		vec2 get_emt_sz_max() const override { return emt_sz_max; }
		void set_emt_sz_max(const vec2& v) override { emt_sz_max = v; }
		vec3 get_emt_rot_min() const override { return emt_rot_min; }
		void set_emt_rot_min(const vec3& v) override { emt_rot_min = v; }
		vec3 get_emt_rot_max() const override { return emt_rot_max; }
		void set_emt_rot_max(const vec3& v) override { emt_rot_max = v; }
		vec3 get_emt_mov_dir_min() const override { return emt_mov_dir_min; }
		void set_emt_mov_dir_min(const vec3& v) override { emt_mov_dir_min = v; }
		vec3 get_emt_mov_dir_max() const override { return emt_mov_dir_max; }
		void set_emt_mov_dir_max(const vec3& v) override { emt_mov_dir_max = v; }
		float get_emt_mov_sp_min() const override { return emt_mov_sp_min; }
		void set_emt_mov_sp_min(float v) override { emt_mov_sp_min = v; }
		float get_emt_mov_sp_max() const override { return emt_mov_sp_max; }
		void set_emt_mov_sp_max(float v) override { emt_mov_sp_max = v; }
		vec3 get_emt_rot_sp_min() const override { return emt_rot_sp_min; }
		void set_emt_rot_sp_min(const vec3& v) override { emt_rot_sp_min = v; }
		vec3 get_emt_rot_sp_max() const override { return emt_rot_sp_max; }
		void set_emt_rot_sp_max(const vec3& v) override { emt_rot_sp_max = v; }
		vec4 get_emt_hsva_min() const override { return emt_hsva_min; }
		void set_emt_hsva_min(const vec4& v) override { emt_hsva_min = v; }
		vec4 get_emt_hsva_max() const override { return emt_hsva_max; }
		void set_emt_hsva_max(const vec4& v) override { emt_hsva_max = v; }
		uint get_emt_ttl_min() const override { return emt_ttl_min; }
		void set_emt_ttl_min(uint v) override { emt_ttl_min = v; }
		uint get_emt_ttl_max() const override { return emt_ttl_max; }
		void set_emt_ttl_max(uint v) override { emt_ttl_max = v; }
		uint get_ptc_num_max() const override { return ptc_num_max; }
		void set_ptc_num_max(uint v) override { ptc_num_max = v; }

		void apply_src();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		void draw(sRendererPtr s_renderer) override;
	};
}
