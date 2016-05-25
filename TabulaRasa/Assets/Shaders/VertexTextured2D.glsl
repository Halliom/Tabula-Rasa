#version 330

in vec3 position;
in vec2 texCoord;

uniform mat4 g_ProjectionMatrix;
uniform vec3 g_PositionOffset;
uniform vec4 g_Color;

out vec4 frag_color;
out vec2 frag_texCoord;

void main()
{
	frag_color = g_Color;
	frag_texCoord = texCoord;
	
	gl_Position = g_ProjectionMatrix * vec4(position + g_PositionOffset, 1.0);
}