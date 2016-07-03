#version 330

uniform sampler2D textureSampler0;

in vec4 frag_color;
in vec2 frag_texCoord;

out vec4 FragColor;

void main()
{
	FragColor = texture(textureSampler0, frag_texCoord) * frag_color;
}