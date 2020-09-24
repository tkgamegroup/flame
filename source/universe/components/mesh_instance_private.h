#pragma once

#include <flame/universe/components/mesh_instance.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
		struct Mesh;
	}

	struct cNodePrivate;

	struct cMeshInstanceBridge : cMeshInstance
	{
		void set_src(const char* src) override;
	};

	struct cMeshInstancePrivate : cMeshInstanceBridge // R ~ on_*
	{
		std::string src;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref
		int model_index = -1;
		int mesh_index = -1;
		graphics::Mesh* mesh = nullptr;

		bool cast_shadow = true;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		int get_mesh_index() const override { return mesh_index; }
		void set_mesh_index(int id) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void on_gain_canvas();

		void get_mesh();

		void draw(graphics::Canvas* canvas); // R
	};

	inline void cMeshInstanceBridge::set_src(const char* src)
	{
		((cMeshInstancePrivate*)this)->set_src(src);
	}
}
