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
	vec4 textureColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (texCoord != vec2(0.0, 0.0))
	{
		vec3 ssao = vec3(texture(textureSampler3, frag_texCoord).r);
		vec4 ambientLighting = vec4(ssao, 1.0);
		frag_color = texture(textureSampler4, texCoord) * ambientLighting;
	}
	//frag_color = vec4(vec3(texture(textureSampler1, frag_texCoord).rgb), 1.0);
}