#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cSky : Component
	{
		inline static auto type_name = "flame::cSky";
		inline static auto type_hash = ch(type_name);

		cSky() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_box_texture() const = 0;
		virtual void set_box_texture(const char* name) = 0;
		virtual const char* get_irr_texture() const = 0;
		virtual void set_irr_texture(const char* name) = 0;
		virtual const char* get_rad_texture() const = 0;
		virtual void set_rad_texture(const char* name) = 0;

		FLAME_UNIVERSE_EXPORTS static cSky* create();
	};
}
