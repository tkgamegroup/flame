#pragma once

#include "../component.h"

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

		virtual const wchar_t* get_src() const = 0;
		virtual void set_src(const wchar_t* src) = 0;
		virtual uint get_sub_index() const = 0;
		virtual void set_sub_index(uint idx) = 0;

		virtual bool get_cast_shadow() const = 0;
		virtual void set_cast_shadow(bool v) = 0;

		virtual ShadingFlags get_shading_flags() const = 0;
		virtual void set_shading_flags(ShadingFlags flags) = 0;

		FLAME_UNIVERSE_EXPORTS static cMesh* create(void* parms = nullptr);
	};
}
