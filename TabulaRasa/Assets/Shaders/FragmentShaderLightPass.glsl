#version 330

uniform sampler2D textureSampler0; // Position
uniform sampler2D textureSampler1; // Normal
uniform sampler2D textureSampler2; // TexCoord
uniform sampler2D textureSampler3; // SSAO
uniform sampler2D textureSampler4; // Texture Atlas

in vec2 frag_texCoord;

out vec4 frag_color;

void main(void)
{
	vec2 texCoord = texture(textureSampler2, frag_texCoord).xy;
	vec3 ssao = vec3(texture(textureSampler3, frag_texCoord).r);
	vec4 ambientLighting = vec4(ssao, 1.0);
	vec4 textureColor = texture(textureSampler4, texCoord) * ambientLighting;
    frag_color = textureColor;
	//frag_color = texture(textureSampler2, frag_texCoord);
}