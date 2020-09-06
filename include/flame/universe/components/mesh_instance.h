#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cMeshInstance : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cMeshInstance";
		inline static auto type_hash = ch(type_name);

		cMeshInstance() :
			Component(type_name, type_hash)
		{
		}

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		virtual int get_mesh_index() const = 0;
		virtual void set_mesh_index(int id) = 0;

		FLAME_UNIVERSE_EXPORTS static cMeshInstance* create();
	};
}
