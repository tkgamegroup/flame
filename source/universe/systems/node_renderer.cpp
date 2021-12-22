#include "node_renderer_private.h"

namespace flame
{
	int sNodeRendererPrivate::set_mesh_res(int idx, graphics::Mesh* mesh)
	{
		return -1;
	}

	int sNodeRendererPrivate::find_mesh_res(graphics::Mesh* mesh) const
	{
		return -1;
	}

	uint sNodeRendererPrivate::add_mesh_transform(const mat4& mat, const mat3& nor)
	{
		return 0;
	}

	uint sNodeRendererPrivate::add_mesh_armature(const mat4* bones, uint count)
	{
		return 0;
	}

	void sNodeRendererPrivate::draw_mesh(uint id, uint mesh_id, uint skin, ShadingFlags flags)
	{

	}

	void sNodeRendererPrivate::update()
	{

	}

	struct sNodeRendererCreatePrivate : sNodeRenderer::Create
	{
		sNodeRendererPtr operator()() override
		{
			return new sNodeRendererPrivate();
		}
	}sNodeRenderer_create_private;
	sNodeRenderer::Create& sNodeRenderer::create = sNodeRenderer_create_private;
}
