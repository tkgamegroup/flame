#pragma once

#include "particle_emitter.h"
#include "node_private.h"

namespace flame
{
	struct cParticleEmitterPrivate : cParticleEmitter, NodeDrawer, NodeMeasurer
	{
		struct Particle
		{
			vec2 base_sz;
			vec4 base_hsva;
			int base_ttl;

			vec3 pos;
			vec2 sz;
			vec3 rot;
			vec4 col;

			vec3 mov_sp;
			vec3 rot_sp;
			int ttl;
		};

		std::filesystem::path img_src;
		std::string tile_name;

		AABB bounds = { vec3(0.f), vec3(0.f) };

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
		uint emt_num = 3;
		uint emt_num_rand = 0;
		vec2 emt_sz = vec2(0.1f);
		vec2 emt_sz_rand = vec2(0.f);
		vec4 ptc_sz_ttl = vec4(0.f);
		vec3 emt_rot = vec3(0.f);
		vec3 emt_rot_rand = vec3(0.f);
		vec3 emt_mov_dir = vec3(0.f);
		vec3 emt_mov_dir_rand = vec3(0.f);
		float emt_mov_sp = 0.f;
		float emt_mov_sp_rand = 0.f;
		vec3 emt_rot_sp = vec3(0.f);
		vec3 emt_rot_sp_rand = vec3(0.f);
		vec4 emt_hsva = vec4(0.f, 0.f, 1.f, 1.f);
		vec4 emt_hsva_rand = vec4(0.f);
		vec4 ptc_alpha_ttl = vec4(0.f);
		uint ptc_ttl = 120;
		uint ptc_ttl_rand = 0;
		uint ptc_num_max = 0xffff;

		uint emt_tick = 0;

		const wchar_t* get_img() const override { return img_src.c_str(); }
		void set_img(const std::filesystem::path& src);
		void set_img(const wchar_t* src) override { set_img(std::filesystem::path(src)); }
		const char* get_tile() const override { return tile_name.c_str(); }
		void set_tile(std::string_view name);
		void set_tile(const char* name) override { set_tile(std::string(name)); }

		AABB get_bounds() const override { return bounds; }
		void set_bounds(const AABB& v) override;

		uint get_emt_intv() const override { return emt_intv; }
		void set_emt_intv(uint v) { emt_intv = v; }
		float get_emt_prob() const override { return emt_prob; }
		void set_emt_prob(float v) override { emt_prob = v; }
		uint get_emt_num() const override { return emt_num; }
		void set_emt_num(uint v) override { emt_num = v; }
		uint get_emt_num_rand() const override { return emt_num_rand; }
		void set_emt_num_rand(uint v) override { emt_num_rand = v; }
		vec2 get_emt_sz() const override { return emt_sz; }
		void set_emt_sz(const vec2& v) override { emt_sz = v; }
		vec2 get_emt_sz_rand() const override { return emt_sz_rand; }
		void set_emt_sz_rand(const vec2& v) override { emt_sz_rand = v; }
		vec4 get_ptc_sz_ttl() const override { return ptc_sz_ttl; }
		void set_ptc_sz_ttl(const vec4& v) override { ptc_sz_ttl = v; }
		vec3 get_emt_rot() const override { return emt_rot; }
		void set_emt_rot(const vec3& v) override { emt_rot = v; }
		vec3 get_emt_rot_rand() const override { return emt_rot_rand; }
		void set_emt_rot_rand(const vec3& v) override { emt_rot_rand = v; }
		vec3 get_emt_mov_dir() const override { return emt_mov_dir; }
		void set_emt_mov_dir(const vec3& v) override { emt_mov_dir = v; }
		vec3 get_emt_mov_dir_rand() const override { return emt_mov_dir_rand; }
		void set_emt_mov_dir_rand(const vec3& v) override { emt_mov_dir_rand = v; }
		float get_emt_mov_sp() const override { return emt_mov_sp; }
		void set_emt_mov_sp(float v) override { emt_mov_sp = v; }
		float get_emt_mov_sp_rand() const override { return emt_mov_sp_rand; }
		void set_emt_mov_sp_rand(float v) override { emt_mov_sp_rand = v; }
		vec3 get_emt_rot_sp() const override { return emt_rot_sp; }
		void set_emt_rot_sp(const vec3& v) override { emt_rot_sp = v; }
		vec3 get_emt_rot_sp_rand() const override { return emt_rot_sp_rand; }
		void set_emt_rot_sp_rand(const vec3& v) override { emt_rot_sp_rand = v; }
		vec4 get_emt_hsva() const override { return emt_hsva; }
		void set_emt_hsva(const vec4& v) override { emt_hsva = v; }
		vec4 get_emt_hsva_rand() const override { return emt_hsva_rand; }
		void set_emt_hsva_rand(const vec4& v) override { emt_hsva_rand = v; }
		vec4 get_ptc_alpha_ttl() const override { return ptc_alpha_ttl; }
		void set_ptc_alpha_ttl(const vec4& v) override { ptc_alpha_ttl = v; }
		uint get_ptc_ttl() const override { return ptc_ttl; }
		void set_ptc_ttl(uint v) override { ptc_ttl = v; }
		uint get_ptc_ttl_rand() const override { return ptc_ttl_rand; }
		void set_ptc_ttl_rand(uint v) override { ptc_ttl_rand = v; }
		uint get_ptc_num_max() const override { return ptc_num_max; }
		void set_ptc_num_max(uint v) override { ptc_num_max = v; }

		void apply_src();

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;

		void draw(sRendererPtr s_renderer, bool, bool) override;
		bool measure(AABB* b) override;
	};
}
