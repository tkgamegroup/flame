#pragma once

#include "../component.h"

namespace flame
{
	struct Particle
	{
		vec3 pos;
		vec2 size;
		vec3 vel;
		float rot;
		uvec4 col;
		float time_max;
		float time;
		float rnd;
	};

	// Reflect ctor
	struct cParticleSystem : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		std::filesystem::path material_name;
		// Reflect
		virtual void set_material_name(const std::filesystem::path& material_name) = 0;
		// Reflect
		uvec2 texture_tiles = uvec2(1U);
		// Reflect
		ivec2 texture_tiles_range = ivec2(0, -1);
		// Reflect
		float particle_life_time = 5.f;
		// Reflect
		float particle_speed = 5.f;
		// Reflect
		vec2 particle_size = vec2(1.f);
		// Reflect
		float particle_rotation_start = 0.f;
		// Reflect
		float particle_rotation_end = 360.f;
		// Reflect
		cvec4 particle_col = cvec4(255);
		// Reflect hash=Sphere|Pie|Cone
		uint emitt_type = "Sphere"_h;
		// Reflect
		float emitt_duration = 0.f; // <0 to disable emittion
		// Reflect
		uint emitt_num = 10; // per second
		// Reflect
		uint emitt_start_num = 0;
		// Reflect
		vec3 emitt_rotation = vec3(0.f);
		// Reflect
		float emitt_offset_start = 0.f;
		// Reflect
		float emitt_offset_end = 0.f;
		// Reflect
		float emitt_bitangent_offset_start = 0.f;
		// Reflect
		float emitt_bitangent_offset_end = 0.f;
		// Reflect
		virtual void set_emitt_rotation(const vec3& r) = 0;
		// Reflect
		float emitt_angle_start = -45.f;
		// Reflect
		float emitt_angle_end = 45.f;
		// Reflect hash=Billboard|HorizontalBillboard|VerticalBillboard
		uint render_type = "Billboard"_h;

		graphics::MaterialPtr material = nullptr;
		int material_res_id = -1;

		virtual std::vector<Particle> get_particles() = 0;
		virtual void set_particles(const std::vector<Particle>& pts) = 0;

		struct Create
		{
			virtual cParticleSystemPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
