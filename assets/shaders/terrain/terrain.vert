layout(location = 0) out flat uint o_id;
layout(location = 1) out flat uint o_matid;
layout(location = 2) out	  vec2 o_uv;

void main(void)
{
	o_id = gl_InstanceIndex >> 24;
	uint tile = gl_InstanceIndex & 0xffff;
	o_matid = (gl_InstanceIndex >> 16) & 0xff;

	vec2 vs[4] = { vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0) };

	uvec2 blocks = instance.terrains[o_id].blocks;
	vec2 v = vec2((tile % blocks.x), (tile / blocks.x)) + vs[gl_VertexIndex];
	v /= blocks;

	o_uv = v;
	gl_Position = instance.terrains[o_id].mat * vec4(vec3(v.x, 0.0, v.y) * instance.terrains[o_id].extent, 1.0);
}
