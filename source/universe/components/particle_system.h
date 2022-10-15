#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cParticleSystem : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		float particle_life_time = 5.f;
		/// Reflect
		float particle_speed = 5.f;
		/// Reflect
		vec2 particle_ext = vec2(1.f);
		/// Reflect hash=sphere|pie|cone
		uint emitt_type = "sphere"_h;
		/// Reflect
		uint emitt_num = 10; // per second
		/// Reflect
		vec3 emitt_rotation = vec3(0.f);
		/// Reflect
		float emitt_angle = 45.f;

		/// Reflect
		std::filesystem::path material_name;
		/// Reflect
		virtual void set_material_name(const std::filesystem::path& material_name) = 0;

		graphics::MaterialPtr material = nullptr;
		int material_res_id = -1;

		struct Create
		{
			virtual cParticleSystemPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
