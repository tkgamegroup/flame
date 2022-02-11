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
		std::filesystem::path model_name;
		/// Reflect
		virtual void set_model_name(const std::filesystem::path& path) = 0;

		/// Reflect
		std::wstring animation_names;
		/// Reflect
		virtual void set_animation_names(const std::wstring& paths) = 0;

		int playing_id = -1;
		int playing_frame = -1;
		float playing_speed = 1.f;
		bool loop = true;

		virtual void play(uint id) = 0;
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
