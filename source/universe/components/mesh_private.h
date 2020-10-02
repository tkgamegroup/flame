#pragma once

#include <flame/universe/components/mesh.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cNodePrivate;

	struct cMeshBridge : cMesh
	{
		void set_src(const char* src) override;
	};

	struct cMeshPrivate : cMeshBridge // R ~ on_*
	{
		int model_id = -1;
		int mesh_id = -1;

		std::string src;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		bool cast_shadow = true;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

		int get_model_id() const override { return model_id; }
		void set_model_id(int id) override;
		int get_mesh_id() const override { return mesh_id; }
		void set_mesh_id(int id) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void apply_src();

		void on_gain_canvas();

		void draw(graphics::Canvas* canvas); // R
	};

	inline void cMeshBridge::set_src(const char* src)
	{
		((cMeshPrivate*)this)->set_src(src);
	}
}
