#version 400
layout(location = 0) in vec3 position;
layout(location = 2) in uvec3 offset;
//in vec3 in_color;

uniform mat4 g_ProjectionMatrix;
uniform mat4 g_ViewMatrix;
uniform mat4 g_WorldTransformMatrix;

out vec4 frag_color;

void main()
{
	gl_Position = g_ProjectionMatrix * g_ViewMatrix * g_WorldTransformMatrix * vec4(position + vec3(offset), 1.0f);
	frag_color = vec4(((gl_VertexID + 0) % 3) / 3.0, ((gl_VertexID + 1) % 3) / 3.0, ((gl_VertexID + 2) % 3) / 3.0, 1.0);
}