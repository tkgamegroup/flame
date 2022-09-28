#include "node_private.h"
#include "nav_mesh_private.h"

namespace flame
{
	struct cNavMeshCreate : cNavMesh::Create
	{
		cNavMeshPtr operator()(EntityPtr e) override
		{
			return new cNavMeshPrivate();
		}
	}cNavMesh_create;
	cNavMesh::Create& cNavMesh::create = cNavMesh_create;
}
