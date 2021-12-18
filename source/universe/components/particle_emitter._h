#pragma once

#include "../component.h"

namespace flame
{
	struct cParticleEmitter : Component
	{
		inline static auto type_name = "flame::cParticleEmitter";
		inline static auto type_hash = ch(type_name);

		cParticleEmitter() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_img() const = 0;
		virtual void set_img(const wchar_t* src) = 0;
		virtual const char* get_tile() const = 0;
		virtual void set_tile(const char* name) = 0;

		virtual AABB get_bounds() const = 0;
		virtual void set_bounds(const AABB& v) = 0;

		virtual uint get_emt_intv() const = 0;
		virtual void set_emt_intv(uint v) = 0;
		virtual float get_emt_prob() const = 0;
		virtual void set_emt_prob(float v) = 0;
		virtual uint get_emt_num() const = 0;
		virtual void set_emt_num(uint v) = 0;
		virtual uint get_emt_num_rand() const = 0;
		virtual void set_emt_num_rand(uint v) = 0;
		virtual vec2 get_emt_sz() const = 0;
		virtual void set_emt_sz(const vec2& v) = 0;
		virtual vec2 get_emt_sz_rand() const = 0;
		virtual void set_emt_sz_rand(const vec2& v) = 0;
		virtual vec4 get_ptc_sz_ttl() const = 0;
		virtual void set_ptc_sz_ttl(const vec4& v) = 0;
		virtual vec3 get_emt_rot() const = 0;
		virtual void set_emt_rot(const vec3& v) = 0;
		virtual vec3 get_emt_rot_rand() const = 0;
		virtual void set_emt_rot_rand(const vec3& v) = 0;
		virtual vec3 get_emt_mov_dir() const = 0;
		virtual void set_emt_mov_dir(const vec3& v) = 0;
		virtual vec3 get_emt_mov_dir_rand() const = 0;
		virtual void set_emt_mov_dir_rand(const vec3& v) = 0;
		virtual float get_emt_mov_sp() const = 0;
		virtual void set_emt_mov_sp(float v) = 0;
		virtual float get_emt_mov_sp_rand() const = 0;
		virtual void set_emt_mov_sp_rand(float v) = 0;
		virtual vec3 get_emt_rot_sp() const = 0;
		virtual void set_emt_rot_sp(const vec3& v) = 0;
		virtual vec3 get_emt_rot_sp_rand() const = 0;
		virtual void set_emt_rot_sp_rand(const vec3& v) = 0;
		virtual vec4 get_emt_hsva() const = 0;
		virtual void set_emt_hsva(const vec4& v) = 0;
		virtual vec4 get_emt_hsva_rand() const = 0;
		virtual void set_emt_hsva_rand(const vec4& v) = 0;
		virtual vec4 get_ptc_alpha_ttl() const = 0;
		virtual void set_ptc_alpha_ttl(const vec4& v) = 0;
		virtual uint get_ptc_ttl() const = 0;
		virtual void set_ptc_ttl(uint v) = 0;
		virtual uint get_ptc_ttl_rand() const = 0;
		virtual void set_ptc_ttl_rand(uint v) = 0;
		virtual uint get_ptc_num_max() const = 0;
		virtual void set_ptc_num_max(uint v) = 0;

		FLAME_UNIVERSE_EXPORTS static cParticleEmitter* create(void* parms = nullptr);
	};
}
