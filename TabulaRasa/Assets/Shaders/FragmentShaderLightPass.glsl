#version 330

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTexCoord;

in vec2 frag_texCoord;

out vec4 frag_color;

void main(void)
{
	frag_color = texture2D(gNormal, frag_texCoord);
}