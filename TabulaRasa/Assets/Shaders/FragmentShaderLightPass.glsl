#version 330

uniform sampler2D textureSampler0; // Position
uniform sampler2D textureSampler1; // Normal
uniform sampler2D textureSampler2; // TexCoord

in vec2 frag_texCoord;

out vec4 frag_color;

void main(void)
{
	frag_color = texture2D(textureSampler0, frag_texCoord);
}