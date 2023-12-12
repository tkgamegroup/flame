#pragma once

#include "particle_system.h"

namespace flame
{
	struct cParticleSystemPrivate : cParticleSystem
	{
		bool dirty = true;

		std::list<Particle> particles;
		std::map<uint /*particle id*/, std::list<Particle>> trails;
		float emitt_timer = 0.f;
		float accumulated_num = 0.f;
		float trail_emitt_timer = 0.f;
		mat3 emitt_rotation_mat = mat3(1.f);
		uint next_particle_id = 0;

		uvec2 texture_sheet_size = uvec2(1, 1);
		uvec2 trail_texture_sheet_size = uvec2(1, 1);

		~cParticleSystemPrivate();
		void set_emitt_rotation(const vec3& r) override;
		void set_material_name(const std::filesystem::path& material_name) override;
		void set_trail_material_name(const std::filesystem::path& material_name) override;
		void on_init() override;
		void on_inactive() override;
		void start() override;
		void update() override;

		void reset() override;
		std::vector<Particle> get_particles() override;
		void set_particles(const std::vector<Particle>& pts) override;
		std::vector<Particle> get_trail(uint id) override;
		void set_trail(uint id, const std::vector<Particle>& vts) override;
	};
}
