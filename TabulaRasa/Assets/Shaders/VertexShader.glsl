#version 400
layout(location = 0) in vec3 position;
layout(location = 2) in uvec3 offset;
//in vec3 in_color;

uniform mat4 g_ProjectionViewMatrix;

uniform vec3 g_ChunkOffset;

out vec4 frag_color;

void main()
{
	vec3 transformedPosition = position + vec3(offset) + g_ChunkOffset;
	gl_Position = g_ProjectionViewMatrix * vec4(transformedPosition, 1.0f);
	frag_color = vec4(1.0, 1.0, 1.0, 1.0);
}