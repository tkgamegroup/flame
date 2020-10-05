#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cMesh : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cMesh";
		inline static auto type_hash = ch(type_name);

		cMesh() :
			Component(type_name, type_hash)
		{
		}

		virtual int get_model_id() const = 0;
		virtual void set_model_id(int id) = 0;
		virtual int get_mesh_id() const = 0;
		virtual void set_mesh_id(int id) = 0;

		virtual const char* get_src() const = 0;
		virtual void set_src(const char* src) = 0;

		virtual bool get_cast_shadow() const = 0;
		virtual void set_cast_shadow(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static cMesh* create();
	};
}