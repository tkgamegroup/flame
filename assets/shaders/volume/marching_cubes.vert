layout(location = 0) out flat uint o_id;
layout(location = 1) out flat uvec3 o_bidx; // block index (in x, y, z)
layout(location = 2) out flat uvec3 o_cidx; // cell index (in x, y, z)

void main(void)
{
	o_id = gl_InstanceIndex >> 24;
	uint bidx = gl_InstanceIndex & 0xffff;
	uvec3 blocks = instance.volumes[o_id].blocks;
	uint cells = instance.volumes[o_id].cells;
	uint cells2 = cells * cells;
	uint blocks_xy = blocks.x * blocks.y;
	o_bidx.z = bidx / blocks_xy;
	bidx = bidx % blocks_xy;
	o_bidx.y = bidx / blocks.x;
	o_bidx.x = bidx % blocks.x;

	uint cidx = gl_VertexIndex;
	o_cidx.z = cidx / cells2;
	cidx = cidx % cells2;
	o_cidx.y = cidx / cells;
	o_cidx.x = cidx % cells;

	gl_Position = vec4(0.0, 0.0, 0.0, 1.0); // not used
}
