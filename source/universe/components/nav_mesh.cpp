#include "node_private.h"
#include "nav_mesh_private.h"
#include "../systems/scene_private.h"

namespace flame
{
	void cNavMeshPrivate::set_filename(const std::filesystem::path& _filename) 
	{
		if (filename == _filename)
			return;
		filename = _filename;

		sScene::instance()->navmesh_load(filename);

		data_changed("filename"_h);
	}

	struct cNavMeshCreate : cNavMesh::Create
	{
		cNavMeshPtr operator()(EntityPtr e) override
		{
			return new cNavMeshPrivate();
		}
	}cNavMesh_create;
	cNavMesh::Create& cNavMesh::create = cNavMesh_create;
}
