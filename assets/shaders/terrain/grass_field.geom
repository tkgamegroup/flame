#include "../math.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location = 0) in flat uint i_id[];
layout(location = 1) in flat uint i_matid[];
layout(location = 2) in vec2 i_uv[];
layout(location = 3) in vec3 i_normal[];
layout(location = 4) in vec3 i_tangent[];
layout(location = 5) in vec3 i_coordw[];

layout(location = 0) out vec3 o_normal;
layout(location = 1) out vec3 o_color;
layout(location = 2) out vec3 o_coordw;

void main()
{
	vec3 p = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0;
	vec3 iN = (i_normal[0] + i_normal[1] + i_normal[2]) / 3.0;
	vec3 iT = (i_tangent[0] + i_tangent[1] + i_tangent[2]) / 3.0;
	vec3 iB = cross(iT, iN);

	if (dot(iN, vec3(0, 1, 0)) > 0.9)
	{
		mat3 mat = rotation(iN, rand(p) * PI) *
			rotation(iT, rand(p.zzx) * 0.25 * PI) * mat3(iT, iN, iB);

		iT = mat[0];
		iN = mat[1];
		iB = mat[2];

		vec3 oN = iB;
		if (dot(oN, scene.camera_dir) > 0.0)
			oN = -oN;

		float width = (rand(p.xzy) * 2 - 1) * 0.02 + 0.05;
		float height = (rand(p.zyx) * 2 - 1) * 0.3 + 0.5;
		
		o_normal = oN;
		o_color = vec3(0.49, 0.36, 0.26);
		o_coordw = p - width * iT;
		gl_Position = scene.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
	
		o_normal = oN;
		o_color = vec3(0.49, 0.36, 0.26);
		o_coordw = p + width * iT;
		gl_Position = scene.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();
	
		o_normal = oN;
		o_color = vec3(0.49, 0.36, 0.26);
		o_coordw = p + height * iN;
		gl_Position = scene.proj_view * vec4(o_coordw, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}
