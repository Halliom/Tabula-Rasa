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
	vec2 texCoord = texture2D(textureSampler2, frag_texCoord).xy;
	vec4 textureColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (texCoord != vec2(0.0, 0.0))
	{
		float ssao = texture2D(textureSampler3, frag_texCoord).r;
		vec4 ambientLighting = vec4(ssao, ssao, ssao, 1.0);
		frag_color = texture2D(textureSampler4, texCoord) * ambientLighting;
	}
}