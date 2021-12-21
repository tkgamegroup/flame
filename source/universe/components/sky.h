#pragma once

#include "../component.h"

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

		virtual const wchar_t* get_box_texture_path() const = 0;
		virtual void set_box_texture_path(const wchar_t* path) = 0;
		virtual const wchar_t* get_irr_texture_path() const = 0;
		virtual void set_irr_texture_path(const wchar_t* path) = 0;
		virtual const wchar_t* get_rad_texture_path() const = 0;
		virtual void set_rad_texture_path(const wchar_t* path) = 0;
		virtual const wchar_t* get_lut_texture_path() const = 0;
		virtual void set_lut_texture_path(const wchar_t* path) = 0;

		virtual float get_intensity() const = 0;
		virtual void set_intensity(float i) = 0;

		FLAME_UNIVERSE_EXPORTS static cSky* create();
	};
}
