#pragma once

#include "../component.h"

namespace flame
{
	struct cArmature : Component
	{
		inline static auto type_name = "flame::cArmature";
		inline static auto type_hash = ch(type_name);

		std::filesystem::path model_path;
		std::wstring animation_paths;

		int animation_id = -1;
		int frame = -1;

		cArmature() :
			Component(type_name, type_hash)
		{
		}

		virtual void set_model_path(const std::filesystem::path& path) = 0;
		virtual void set_animation_paths(const std::wstring& paths) = 0;

		virtual void play(uint id, float speed, bool loop) = 0;
		virtual void stop() = 0;
		virtual void stop_at(uint id, int frame) = 0;

		FLAME_UNIVERSE_EXPORTS static cArmature* create();
	};
}
