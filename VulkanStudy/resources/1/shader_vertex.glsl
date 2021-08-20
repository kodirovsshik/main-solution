
#version 450

vec3 colors[3] = 
{
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
};

vec2 positions[3] = 
{
	vec2(0, -0.5f),
	vec2(-0.5f, 0.5f),
	vec2(0.5f, 0.5f),
};

layout(location = 0) out vec3 output_color;

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
	output_color = colors[gl_VertexIndex];
}
