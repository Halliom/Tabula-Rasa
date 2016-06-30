#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 color;

uniform mat4 g_ProjectionMatrix;

out vec2 frag_texCoord;
out vec4 frag_color;

void main()
{
	frag_texCoord = texCoord;
	frag_color = color;
	gl_Position = g_ProjectionMatrix * vec4(position, 0.0, 1.0);
}