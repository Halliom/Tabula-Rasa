#version 400
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 dimension;
layout(location = 2) in uint textureCoord;

uniform mat4 g_ProjectionViewMatrix;

uniform vec3 g_ChunkOffset;

out vec4 frag_color;

void main()
{
	vec3 transformedPosition = position + g_ChunkOffset;
	gl_Position = g_ProjectionViewMatrix * vec4(transformedPosition, 1.0f);
	frag_color = vec4(textureCoord, 0.0, 0.0, 1.0);
}