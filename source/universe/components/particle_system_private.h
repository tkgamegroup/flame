#pragma once

#include "particle_system.h"

namespace flame
{
	struct cParticleSystemPrivate : cParticleSystem
	{
		bool dirty = true;

		std::list<Particle> particles;
		float emitt_timer = 0.f;
		float accumulated_num = 0.f;
		mat3 emitt_rotation_mat = mat3(1.f);
		uvec2 color_map_tiles = uvec2(1);

		~cParticleSystemPrivate();
		void set_emitt_rotation(const vec3& r) override;
		void set_material_name(const std::filesystem::path& material_name) override;
		void on_init() override;
		void on_inactive() override;
		void start() override;
		void update() override;

		std::vector<Particle> get_particles() override;
		void set_particles(const std::vector<Particle>& pts) override;
	};
}
