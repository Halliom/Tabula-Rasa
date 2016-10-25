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

// Since the texture atlas contains a 16x16 grid of 'tiles'
const vec2 TILE_OFFSET = vec2(1.0 / 16.0, 1.0 / 16.0);

vec2 getAtlasOffset()
{
	float xCoord = float(textureCoord % uint(16));
	float yCoord = float(textureCoord / uint(16));
	return vec2(xCoord, yCoord) * TILE_OFFSET;
}

void main()
{
	vec4 viewPos = g_ViewMatrix * g_ModelMatrix * vec4(position, 1.0);
	frag_position = viewPos.xyz;
	gl_Position = g_ProjectionMatrix * viewPos;

	frag_atlasOffset = getAtlasOffset();
	frag_texCoord = dimension;

	mat3 normalMatrix = transpose(inverse(mat3(g_ViewMatrix * g_ModelMatrix)));
	frag_normal = normalize(normalMatrix * normal);
}
