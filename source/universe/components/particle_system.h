#pragma once

#include "../component.h"

namespace flame
{
	enum ParticleEmittType
	{
		ParticleEmittSphere,
		ParticleEmittPie,
		ParticleEmittCone
	};

	enum ParticleRenderType
	{
		ParticleBillboard,
		ParticleHorizontalBillboard,
		ParticleVerticalBillboard
	};

	struct Particle
	{
		vec3 pos;
		vec2 size;
		vec3 vel;
		float ang;
		uvec4 col;
		float life_time;
		float time;
		uint id;
	};

	// Reflect ctor
	struct cParticleSystem : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		float particle_life_time_min = 5.f;
		// Reflect
		float particle_life_time_max = 5.f;
		// Reflect
		float particle_speed_min = 5.f;
		// Reflect
		float particle_speed_max = 5.f;
		// Reflect
		vec2 particle_size = vec2(1.f);
		// Reflect
		float particle_angle_min = 0.f;
		// Reflect
		float particle_angle_max = 360.f;
		// Reflect
		cvec4 particle_color = cvec4(255);
		// Reflect
		ParticleEmittType emitt_type = ParticleEmittSphere;
		// Reflect
		float emitt_duration = 0.f; // <0 to disable emittion and reset timer
		// Reflect
		float emitt_num = 10.f; // per second
		// Reflect
		uint emitt_start_num = 0;
		// Reflect
		vec3 emitt_rotation = vec3(0.f);
		// Reflect
		float emitt_offset_min = 0.f;
		// Reflect
		float emitt_offset_max = 0.f;
		// Reflect
		float emitt_bitangent_offset_min = 0.f;
		// Reflect
		float emitt_bitangent_offset_max = 0.f;
		// Reflect
		virtual void set_emitt_rotation(const vec3& r) = 0;
		// Reflect
		float emitt_angle_min = -45.f;
		// Reflect
		float emitt_angle_max = 45.f;
		// Reflect
		ParticleRenderType render_type = ParticleBillboard;
		// Reflect
		std::filesystem::path material_name;
		// Reflect
		virtual void set_material_name(const std::filesystem::path& material_name) = 0;
		// Reflect
		bool enable_trail = false;
		// Reflect
		float trail_emitt_tick = 1.f;
		// Reflect
		float trail_life_time_min = 5.f;
		// Reflect
		float trail_life_time_max = 5.f;
		// Reflect
		vec2 trail_size = vec2(1.f);
		// Reflect
		cvec4 trail_color = cvec4(255);
		// Reflect
		ParticleRenderType trail_render_type = ParticleBillboard;
		// Reflect
		std::filesystem::path trail_material_name;
		// Reflect
		virtual void set_trail_material_name(const std::filesystem::path& material_name) = 0;

		graphics::MaterialPtr material = nullptr;
		graphics::MaterialPtr trail_material = nullptr;
		int material_res_id = -1;
		int trail_material_res_id = -1;

		virtual void reset() = 0;
		virtual std::vector<Particle> get_particles() = 0;
		virtual void set_particles(const std::vector<Particle>& pts) = 0;
		virtual std::vector<Particle> get_trail(uint id) = 0;
		virtual void set_trail(uint id, const std::vector<Particle>& vts) = 0;

		struct Create
		{
			virtual cParticleSystemPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
