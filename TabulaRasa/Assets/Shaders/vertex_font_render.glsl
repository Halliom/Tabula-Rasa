#version 330
in vec3 position;
in vec2 texCoord;

uniform mat4 g_ProjectionMatrix;
uniform vec3 g_PositionOffset;

out vec3 frag_color;
out vec2 frag_texCoord;

void main()
{
	frag_texCoord = texCoord;
	frag_color = vec3(1.0, 0.0, 0.0);
	gl_Position = g_ProjectionMatrix * vec4(position + g_PositionOffset, 1.0);
}