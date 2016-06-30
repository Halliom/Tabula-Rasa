#version 330

layout(location = 0) in vec3 position;

uniform mat4 g_ProjectionMatrix;
uniform vec3 g_PositionOffset;
uniform vec4 g_Color;

out vec4 frag_color;

void main()
{
	frag_color = g_Color;
	
	gl_Position = g_ProjectionMatrix * vec4(position + g_PositionOffset, 1.0);
}