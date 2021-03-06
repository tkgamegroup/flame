#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cMesh : Component
	{
		inline static auto type_name = "flame::cMesh";
		inline static auto type_hash = ch(type_name);

		cMesh() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		virtual bool get_cast_shadow() const = 0;
		virtual void set_cast_shadow(bool v) = 0;

		virtual void set_animation(const char* name, bool loop = true, uint layer = 0) = 0;

		FLAME_UNIVERSE_EXPORTS static cMesh* create(void* parms = nullptr);
	};
}
