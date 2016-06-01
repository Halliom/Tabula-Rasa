#version 330

uniform sampler2D textureSampler0;

in vec2 frag_texCoord;
in vec4 frag_color;

out vec4 FragColor;

void main()
{
	FragColor = texture2D(textureSampler0, frag_texCoord) * frag_color;
}