layout(location = 1) in vec3 i_view;

layout(location = 0) out vec4 fColor;

void main()
{
	fColor = vec4(pow(vec3(max(dot(normalize(i_view), vec3(0.0, 0.707, 0.707)), 0.0)), vec3(1024.0)) * 1024.0, 1.0);
}
