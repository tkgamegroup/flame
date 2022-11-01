layout (points) in;
layout (triangle_strip, max_vertices = 3) out;

layout(location = 0) in flat uint i_id[];
layout(location = 1) in flat uvec3 i_bidx[]; // block index (in x, y, z)
layout(location = 2) in flat uvec3 i_cidx[]; // cell index (in x, y, z)

const int case_to_numpolys[256] = {
	0, 1, 1, 2, 1, 2, 2, 3,  1, 2, 2, 3, 2, 3, 3, 2,  1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 2, 3, 4, 4, 3,  3, 4, 4, 3, 4, 5, 5, 2,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  
    2, 3, 3, 4, 3, 4, 2, 3,  3, 4, 4, 5, 4, 5, 3, 2,  3, 4, 4, 3, 4, 5, 3, 2,  4, 5, 5, 4, 5, 2, 4, 1,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 2, 4, 3, 4, 3, 5, 2,  
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  3, 4, 4, 3, 4, 5, 5, 4,  4, 3, 5, 2, 5, 4, 2, 1,  
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 2, 3, 3, 2,  3, 4, 4, 5, 4, 5, 5, 2,  4, 3, 5, 4, 3, 2, 4, 1,  
    3, 4, 4, 5, 4, 5, 3, 4,  4, 5, 5, 2, 3, 4, 2, 1,  2, 3, 3, 2, 3, 4, 2, 1,  3, 2, 4, 1, 2, 1, 1, 0
};

void main()
{
	gl_Position = camera.proj_view * vec4(vec3(i_cidx[0]) + vec3(i_bidx[0]) * 64, 1.0);
	EmitVertex();

	gl_Position = camera.proj_view * vec4(vec3(i_cidx[0]) + vec3(0.0, 0.0, 0.2) + vec3(i_bidx[0]) * 64, 1.0);
	EmitVertex();

	gl_Position = camera.proj_view * vec4(vec3(i_cidx[0]) + vec3(0.2, 0.0, 0.0) + vec3(i_bidx[0]) * 64, 1.0);
	EmitVertex();

	EndPrimitive();
}
