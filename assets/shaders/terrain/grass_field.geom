#include "../math.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location = 0) in flat uint i_id[];
layout(location = 1) in flat uint i_matid[];
layout(location = 2) in vec2 i_uv[];
layout(location = 3) in vec3 i_normal[];
layout(location = 4) in vec3 i_tangent[];
layout(location = 5) in vec3 i_coordw[];

layout(location = 0) out uint o_id;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out vec3 o_normal;
layout(location = 3) out vec3 o_color;
layout(location = 4) out vec3 o_coordw;

void main()
{
	vec3 pp = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0;

	vec3 bc;
	bc[0] = rand(pp.zxy);
	bc[1] = rand(pp.zyx) * (1.0 - bc[0]);
	bc[2] = 1.0 - bc[0] - bc[1];
	vec3 p = gl_in[0].gl_Position.xyz * bc[0] + gl_in[1].gl_Position.xyz * bc[1] + gl_in[2].gl_Position.xyz * bc[2];

	vec3 iN = i_normal[0] * bc[0] + i_normal[1] * bc[1] + i_normal[2] * bc[2];
	vec3 iT = i_tangent[0] * bc[0] + i_tangent[1] * bc[1] + i_tangent[2] * bc[2];
	vec3 iB = cross(iT, iN);

	uint tid = i_id[0];
	if (texture(terrain_splash_maps[tid], i_uv[0])[terrain_instances[tid].grass_channel] > 0.5)
	{
	#ifndef BILLBOARD
		iT = camera.right;
		iN = camera.up;
		vec3 oN = -camera.front;
		
		float size = (rand(pp.yzx) * 2 - 1) * 0.25 + 0.9;
		
		o_id = tid;
		o_uv = vec2(1.0, 1.0);
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p + size * iT * 0.5;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
		
		o_id = tid;
		o_uv = vec2(1.0, 0.0);
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p + size * iT * 0.5 + size * iN;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
		
		o_id = tid;
		o_uv = vec2(0.0, 1.0);
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p - size * iT * 0.5;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
		
		o_id = tid;
		o_uv = vec2(0.0, 0.0);
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p - size * iT * 0.5 + size * iN;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
	#else
		mat3 mat = rotation(iN, rand(pp.xyz) * PI) *
			rotation(iT, rand(pp.xzy) * 0.25 * PI) * mat3(iT, iN, iB);

		iT = mat[0];
		iN = mat[1];
		iB = mat[2];

		vec3 oN = iB;
		if (dot(oN, camera.front) > 0.0)
			oN = -oN;

		float width = (rand(pp.yxz) * 2 - 1) * 0.02 + 0.05;
		float height = (rand(pp.yzx) * 2 - 1) * 0.3 + 0.5;
		
		o_id = tid;
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p - width * iT;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
	
		o_id = tid;
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p + width * iT;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
	
		o_id = tid;
		o_normal = oN;
		o_color = vec3(0.25, 0.54, 0.2);
		o_coordw = p + height * iN;
		gl_Position = camera.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();

		EndPrimitive();
	#endif
	}
}
