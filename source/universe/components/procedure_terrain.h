#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cProcedureTerrain : Component
	{
		struct SpawnSetting
		{
			std::filesystem::path prefab_name;
			uint channel;
			uint count;
		};

		// Reflect requires
		cTerrainPtr terrain = nullptr;

		// Reflect
		uvec2 image_size = uvec2(512);
		// Reflect
		virtual void set_image_size(const uvec2& size) = 0;

		// Reflect
		int voronoi_sites_count = 20;
		// Reflect
		virtual void set_voronoi_sites_count(uint count) = 0;
		// Reflect
		int voronoi_layer1_precentage = 50;
		// Reflect
		virtual void set_voronoi_layer1_precentage(uint precentage) = 0;
		// Reflect
		int voronoi_layer2_precentage = 0;
		// Reflect
		virtual void set_voronoi_layer2_precentage(uint precentage) = 0;
		// Reflect
		int voronoi_layer3_precentage = 0;
		// Reflect
		virtual void set_voronoi_layer3_precentage(uint precentage) = 0;

		// Reflect
		int splash_layers = 2;
		// Reflect
		virtual void set_splash_layers(uint layers) = 0;
		// Reflect
		float splash_bar1 = 60.f;
		// Reflect
		virtual void set_splash_bar1(float bar) = 0;
		// Reflect
		float splash_bar2 = 80.f;
		// Reflect
		virtual void set_splash_bar2(float bar) = 0;
		// Reflect
		float splash_bar3 = 90.f;
		// Reflect
		virtual void set_splash_bar3(float bar) = 0;
		// Reflect
		float splash_transition = 4.f;
		// Reflect
		virtual void set_splash_transition(float transition) = 0;

		// Reflect
		std::vector<SpawnSetting> spawn_settings;
		// Reflect
		virtual void set_spawn_settings(const std::vector<SpawnSetting>& settings) = 0;

		struct Create
		{
			virtual cProcedureTerrainPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
