
#version 430

in vec3 color;
out vec4 frag_color;

void main()
{
	frag_color = vec4(1 - color, 1);
}
