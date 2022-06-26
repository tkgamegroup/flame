#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cArmature : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		std::filesystem::path armature_name;
		/// Reflect
		virtual void set_armature_name(const std::filesystem::path& name) = 0;

		/// Reflect
		std::vector<std::pair<std::filesystem::path, std::string>> animation_names;
		/// Reflect
		virtual void set_animation_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names) = 0;

		/// Reflect
		bool loop = true;

		graphics::ModelPtr model = nullptr;
		uint playing_name = 0;
		float playing_time = 0;
		float playing_speed = 1.f;

		virtual void play(uint name, float transition = 0.f) = 0;
		virtual void stop() = 0;

		int instance_id = -1;

		struct Create
		{
			virtual cArmaturePtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
