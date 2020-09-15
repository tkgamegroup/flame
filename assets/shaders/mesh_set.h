struct MeshMatrix
{
	mat4 model;
	mat4 normal;
};

layout (set = 0, binding = 0) buffer readonly MeshMatrices
{
	MeshMatrix mesh_matrices[];
};
