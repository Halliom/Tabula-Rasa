#version 330

uniform sampler2D textureSampler0; // Position
uniform sampler2D textureSampler1; // Normal
uniform sampler2D textureSampler2; // TexCoord
uniform sampler2D textureSampler3; // TexCoord

in vec2 frag_texCoord;

out vec4 frag_color;

void main(void)
{
	vec2 texCoord = texture2D(textureSampler2, frag_texCoord).xy;
	if (texCoord != vec2(0.0, 0.0))
		frag_color = texture2D(textureSampler3, texCoord);
	//frag_color = texture2D(textureSampler1, frag_texCoord);
}