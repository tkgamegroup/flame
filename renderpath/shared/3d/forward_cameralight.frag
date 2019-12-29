layout(location = 1) in vec3 i_normal;

layout(location = 0) out vec4 fColor;

void main()
{
	fColor = vec4(vec3(dot(i_normal, vec3(0.0, 0.0, 1.0))), 1.0);
}
