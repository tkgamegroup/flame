#pragma once

#include "../component.h"

namespace flame
{
	struct cArmature : Component
	{
		inline static auto type_name = "flame::cArmature";
		inline static auto type_hash = ch(type_name);

		cArmature() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_model() const = 0;
		virtual void set_model(const wchar_t* src) = 0;

		virtual const wchar_t* get_animations() const = 0;
		virtual void set_animations(const wchar_t* src) = 0;

		virtual int get_curr_anim() = 0;
		virtual int get_curr_frame() = 0;
		virtual void play(uint id, float speed, bool loop) = 0;
		virtual void stop() = 0;
		virtual void stop_at(uint id, int frame) = 0;

		FLAME_UNIVERSE_EXPORTS static cArmature* create(void* parms = nullptr);
	};
}
