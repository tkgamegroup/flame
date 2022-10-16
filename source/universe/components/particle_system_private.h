#pragma once

#include "particle_system.h"

namespace flame
{
	struct cParticleSystemPrivate : cParticleSystem
	{
		struct Particle
		{
			vec3 pos;
			vec2 ext;
			vec3 vel;
			uvec4 col;
			float time;
			float rnd;
		};

		bool dirty = true;

		float acc_num = 0.f;
		std::list<Particle> particles;

		~cParticleSystemPrivate();
		void on_init() override;

		void set_material_name(const std::filesystem::path& material_name) override;
	};
}
