layout(triangles) in;  
layout(triangle_strip, max_vertices = 3) out;

layout (location = 0) in vec2 inUV[];

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;

void main(void)
{
	vec3 v0 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 v1 = gl_in[0].gl_Position.xyz - gl_in[2].gl_Position.xyz;
	vec3 normal = normalize(cross(v0, v1));

	for(int i = 0; i < gl_in.length(); ++i)  
	{  
		gl_Position = gl_in[i].gl_Position;  
  
		outUV = inUV[i];
		outNormal = normal;
  
		EmitVertex();  
	}  
  
	EndPrimitive();
}
