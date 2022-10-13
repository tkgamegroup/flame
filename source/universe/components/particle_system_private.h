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
			float time;
		};

		bool dirty = true;

		std::list<Particle> particles;

		~cParticleSystemPrivate();
		void on_init() override;

		void set_material_name(const std::filesystem::path& material_name) override;
	};
}
