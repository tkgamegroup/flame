#pragma once

#include "mesh.h"
#include "node_private.h"

namespace flame
{
	struct cMeshPrivate : cMesh, NodeDrawer, NodeMeasurer
	{
		std::filesystem::path src;
		uint sub_index;

		bool cast_shadow = true;

		ShadingFlags shading_flags = ShadingMaterial;

		cNodePrivate* node = nullptr;
		cAnimationPrivate* pani = nullptr;
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

		ShadingFlags get_shading_flags() const override { return shading_flags; }
		void set_shading_flags(ShadingFlags flags) override;

		void apply_src();

		void draw(sRendererPtr s_renderer) override;
		bool measure(AABB* b) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
