#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 dimension;
layout (location = 3) in uint textureCoord;

uniform mat4 g_ProjectionMatrix;
uniform mat4 g_ViewMatrix;
uniform mat4 g_ModelMatrix;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texCoord;
out vec2 frag_atlasOffset;

const float TILE_OFFSET = 1.0 / 16.0;
const vec2 TILE_OFFSETV = vec2(TILE_OFFSET, TILE_OFFSET);

vec2 getAtlasOffset()
{
	vec2 atlasOffset = vec2(float(textureCoord % uint(16)), floor(float(textureCoord) * TILE_OFFSET));
	atlasOffset = atlasOffset * TILE_OFFSETV;
	return atlasOffset;
}

void main()
{
	vec4 viewPos = g_ViewMatrix * g_ModelMatrix * vec4(position, 1.0);
	frag_position = viewPos.xyz;
	
	mat3 normalMatrix = transpose(inverse(mat3(g_ViewMatrix * g_ModelMatrix)));
	frag_normal = normalize(normalMatrix * normal);
	
	frag_atlasOffset = getAtlasOffset();
	frag_texCoord = dimension;
	
	gl_Position = g_ProjectionMatrix * transformedPosition;
}