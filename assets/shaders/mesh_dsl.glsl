struct MeshMatrix
{
	mat4 model;
	mat4 normal;
};

layout (set = MESH_SET, binding = 0) buffer readonly MeshMatrices
{
	MeshMatrix mesh_matrices[];
};
