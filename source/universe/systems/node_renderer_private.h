#pragma once

#include "node_renderer.h"

namespace flame
{
	struct sNodeRendererPrivate : sNodeRenderer
	{
		int set_mesh_res(int idx, graphics::Mesh* mesh) override;
		int find_mesh_res(graphics::Mesh* mesh) const override;

		uint add_mesh_transform(const mat4& mat, const mat3& nor) override;
		uint add_mesh_armature(const mat4* bones, uint count) override;
		void draw_mesh(uint id, uint mesh_id, uint skin, ShadingFlags flags) override;

		void update() override;
	};
}
