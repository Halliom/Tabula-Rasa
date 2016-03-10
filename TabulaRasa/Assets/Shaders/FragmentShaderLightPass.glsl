#version 400

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gTexCoord;

in vec2 frag_texCoord;

out vec4 frag_color;

void main(void)
{
	frag_color = vec4(texture2D(gTexCoord, frag_texCoord).rgb, 1.0);
}