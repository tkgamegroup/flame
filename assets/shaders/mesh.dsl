#ifdef MAKE_DSL
#define MESH_SET 0
#endif

struct MeshMatrix
{
	mat4 transform;
	mat4 normal_matrix;
};

layout (set = MESH_SET, binding = 0) buffer readonly MeshMatrices
{
	MeshMatrix mesh_matrices[];
};
