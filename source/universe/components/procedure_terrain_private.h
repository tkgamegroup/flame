#pragma once

#include "procedure_terrain.h"

namespace flame
{
	struct cProcedureTerrainPrivate : cProcedureTerrain
	{
		~cProcedureTerrainPrivate();

		graphics::ImagePtr height_map = nullptr;
		graphics::ImagePtr splash_map = nullptr;

		void set_image_size(const uvec2& size) override;

		void set_voronoi_sites_count(uint count) override;
		void set_voronoi_layer1_precentage(uint precentage) override;
		void set_voronoi_layer2_precentage(uint precentage) override;
		void set_voronoi_layer3_precentage(uint precentage) override;

		void set_splash_layers(uint layers) override;
		void set_splash_bar1(float bar) override;
		void set_splash_bar2(float bar) override;
		void set_splash_bar3(float bar) override;
		void set_splash_transition(float transition) override;

		void set_spawn_settings(const std::vector<SpawnSetting>& settings) override;

		void build_terrain();

		void on_init() override;
	};
}
