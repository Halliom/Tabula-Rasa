#version 400
uniform sampler2D textureSampler0;

in vec2 frag_texCoord; 
in vec2 frag_atlasOffset;

out vec4 fragment_color;

const float TILE_OFFSET = 1.0 / 16.0;

void main()
{
	vec2 texCoord = frag_atlasOffset + (fract(frag_texCoord) * TILE_OFFSET);
	fragment_color = texture2D(textureSampler0, texCoord);
}