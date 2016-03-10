#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 dimension;
layout (location = 2) in vec3 normal;
layout (location = 3) in uint textureCoord;

uniform mat4 g_ProjectionViewMatrix;
uniform vec3 g_ChunkOffset;

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
	vec3 offsetPosition = position + g_ChunkOffset;
	vec4 transformedPosition = g_ProjectionViewMatrix * vec4(offsetPosition, 1.0);
	
	frag_position = transformedPosition.xyz;
	
	//vec4 v4_normal = g_projectionViewMatrix * vec4(normal, 1.0);
	frag_normal = vec3(1.0, 0.0, 0.0);
	
	frag_atlasOffset = getAtlasOffset();
	
	frag_texCoord = dimension;
	
	gl_Position = transformedPosition;
}