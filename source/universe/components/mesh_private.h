#pragma once

#include "mesh.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		std::filesystem::path src;
		uint sub_index;

		bool cast_shadow = true;

		cNodePrivate* node = nullptr;
		cAnimationPrivate* pani = nullptr;
		void* drawer = nullptr;
		void* measurer = nullptr;
		sRendererPrivate* s_renderer = nullptr;

		int mesh_id = -1;
		graphics::Mesh* mesh = nullptr;

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const std::filesystem::path& src);
		void set_src(const wchar_t* src) override { set_src(std::filesystem::path(src)); }
		uint get_sub_index() const override { return sub_index; }
		void set_sub_index(uint idx) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		void apply_src();

		void draw(sRenderer* s_renderer);
		bool measure(AABB* b);

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
