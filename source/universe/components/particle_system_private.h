#pragma once

#include "particle_system.h"

namespace flame
{
	struct cParticleSystemPrivate : cParticleSystem
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

		bool dirty = true;

		std::list<Particle> particles;
		float emitt_timer = 0.f;
		float accumulated_num = 0.f;
		uvec2 tiles = uvec2(1);

		~cParticleSystemPrivate();
		void on_init() override;
		void update() override;

		void set_material_name(const std::filesystem::path& material_name) override;
	};
}
