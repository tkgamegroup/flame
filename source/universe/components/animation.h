#pragma once

#include "../component.h"

namespace flame
{
	struct cAnimation : Component
	{
		inline static auto type_name = "flame::cAnimation";
		inline static auto type_hash = ch(type_name);

		cAnimation() :
			Component(type_name, type_hash)
		{
		}

		virtual const wchar_t* get_model_name() const = 0;
		virtual void set_model_name(const wchar_t* src) = 0;

		virtual const wchar_t* get_src() const = 0;
		virtual void set_src(const wchar_t* src) = 0;

		virtual bool get_loop() const = 0;
		virtual void set_loop(bool l) = 0;

		FLAME_UNIVERSE_EXPORTS static cAnimation* create(void* parms = nullptr);
	};
}
