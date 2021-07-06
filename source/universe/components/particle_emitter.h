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

		virtual uint get_emt_intv() const = 0;
		virtual void set_emt_intv(uint v) = 0;
		virtual float get_emt_prob() const = 0;
		virtual void set_emt_prob(float v) = 0;
		virtual uint get_emt_num_min() const = 0;
		virtual void set_emt_num_min(uint v) = 0;
		virtual uint get_emt_num_max() const = 0;
		virtual void set_emt_num_max(uint v) = 0;
		virtual vec2 get_emt_sz_min() const = 0;
		virtual void set_emt_sz_min(const vec2& v) = 0;
		virtual vec2 get_emt_sz_max() const = 0;
		virtual void set_emt_sz_max(const vec2& v) = 0;
		virtual vec3 get_emt_rot_min() const = 0;
		virtual void set_emt_rot_min(const vec3& v) = 0;
		virtual vec3 get_emt_rot_max() const = 0;
		virtual void set_emt_rot_max(const vec3& v) = 0;
		virtual vec3 get_emt_mov_dir_min() const = 0;
		virtual void set_emt_mov_dir_min(const vec3& v) = 0;
		virtual vec3 get_emt_mov_dir_max() const = 0;
		virtual void set_emt_mov_dir_max(const vec3& v) = 0;
		virtual float get_emt_mov_sp_min() const = 0;
		virtual void set_emt_mov_sp_min(float v) = 0;
		virtual float get_emt_mov_sp_max() const = 0;
		virtual void set_emt_mov_sp_max(float v) = 0;
		virtual vec3 get_emt_rot_sp_min() const = 0;
		virtual void set_emt_rot_sp_min(const vec3& v) = 0;
		virtual vec3 get_emt_rot_sp_max() const = 0;
		virtual void set_emt_rot_sp_max(const vec3& v) = 0;
		virtual vec4 get_emt_hsva_min() const = 0;
		virtual void set_emt_hsva_min(const vec4& v) = 0;
		virtual vec4 get_emt_hsva_max() const = 0;
		virtual void set_emt_hsva_max(const vec4& v) = 0;
		virtual uint get_emt_ttl_min() const = 0;
		virtual void set_emt_ttl_min(uint v) = 0;
		virtual uint get_emt_ttl_max() const = 0;
		virtual void set_emt_ttl_max(uint v) = 0;
		virtual uint get_ptc_num_max() const = 0;
		virtual void set_ptc_num_max(uint v) = 0;

		FLAME_UNIVERSE_EXPORTS static cParticleEmitter* create(void* parms = nullptr);
	};
}
