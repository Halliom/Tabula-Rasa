#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

out vec4 frag_color;

uniform mat4 g_ProjectionMatrix;

void main()
{
	frag_color = color;
	gl_Position = g_ProjectionMatrix * vec4(position, 1.0);
}