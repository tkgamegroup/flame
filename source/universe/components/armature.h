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
		bool loop = true;

		uint playing_name = 0;
		float playing_time = 0;
		float playing_speed = 1.f;

		virtual void bind_animation(uint name_hash, const std::filesystem::path& animation_path) = 0;
		virtual void unbind_animation(uint name_hash) = 0;
		virtual void play(uint name) = 0;
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
