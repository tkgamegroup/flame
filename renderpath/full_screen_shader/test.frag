layout(location = 0) out vec4 fColor;

void main()
{
	fColor = vec4(gl_FragCoord.x / 1000.0, 0.6, 0.7, 1);
}
