#version 400
uniform sampler2D textureSampler0;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gTexCoord;

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texCoord;
in vec2 frag_atlasOffset;

//out vec4 fragment_color;

const float TILE_OFFSET = 1.0 / 16.0;

void main()
{
	vec2 texCoord = frag_atlasOffset + (fract(frag_texCoord) * TILE_OFFSET);
	//fragment_color = texture2D(textureSampler0, texCoord);
	
	gPosition = frag_position;
	gNormal = frag_normal;
	gTexCoord = vec3(texCoord, 0.0);
}