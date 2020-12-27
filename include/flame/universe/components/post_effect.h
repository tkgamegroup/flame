#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cPostEffect : Component
	{
		inline static auto type_name = "flame::cPostEffect";
		inline static auto type_hash = ch(type_name);

		cPostEffect() :
			Component(type_name, type_hash)
		{
		}

		virtual uint get_blur_radius() const = 0;
		virtual void set_blur_radius(uint r) = 0;

		virtual bool get_enable_bloom() const = 0;
		virtual void set_enable_bloom(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static cPostEffect* create();
	};
}
