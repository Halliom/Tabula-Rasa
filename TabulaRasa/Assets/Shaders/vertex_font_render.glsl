#version 430
layout (location = 0) in vec3 position;
layout (location= 1) in vec2 texCoord;

uniform mat4 g_ProjectionMatrix;
uniform mat4 g_WorldTransformMatrix;

out vec3 frag_color;
out vec2 frag_texCoord;

void main()
{
	frag_texCoord = texCoord;
	frag_color = vec3(1.0, 1.0, 1.0);
	gl_Position = g_ProjectionMatrix * vec4(position, 1.0);
}