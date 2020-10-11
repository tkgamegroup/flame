#pragma once

#include <flame/universe/components/mesh.h>

namespace flame
{
	namespace graphics
	{
		struct Mesh;
		struct ArmatureDeformer;
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

		graphics::Mesh* mesh = nullptr;
		graphics::ArmatureDeformer* deformer = nullptr;

		std::string src;

		cNodePrivate* node = nullptr; // R ref
		graphics::Canvas* canvas = nullptr; // R ref

		bool cast_shadow = true;

		const char* get_src() const override { return src.c_str(); }
		void set_src(const std::string& src);

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
