#pragma once

#include "mesh.h"
#include "node_private.h"

namespace flame
{
	struct cMeshPrivate : cMesh
	{
		std::filesystem::path src;
		uint sub_index;
		uint skin = 0;

		bool cast_shadow = true;

		ShadingFlags shading_flags = ShadingMaterial;

		cNodePrivate* node = nullptr;
		cArmaturePrivate* parmature = nullptr;
		int transform_id = -1;
		sRendererPrivate* s_renderer = nullptr;
		uint frame = 0;

		int mesh_id = -1;
		graphics::Mesh* mesh = nullptr;

		const wchar_t* get_src() const override { return src.c_str(); }
		void set_src(const std::filesystem::path& src);
		void set_src(const wchar_t* src) override { set_src(std::filesystem::path(src)); }
		uint get_sub_index() const override { return sub_index; }
		void set_sub_index(uint idx) override;
		uint get_skin() const override { return skin; }
		void set_skin(uint skin) override;

		bool get_cast_shadow() const override { return cast_shadow; }
		void set_cast_shadow(bool v) override;

		ShadingFlags get_shading_flags() const override { return shading_flags; }
		void set_shading_flags(ShadingFlags flags) override;

		void apply_src();

		void draw(sRendererPtr s_renderer, bool shadow_pass) override;
		bool measure(AABB* b) override;

		void on_added() override;
		void on_removed() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
