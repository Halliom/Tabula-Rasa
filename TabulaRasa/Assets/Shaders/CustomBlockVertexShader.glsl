#version 330
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 g_ProjectionMatrix;
uniform mat4 g_ViewMatrix;
uniform mat4 g_ModelMatrix;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texCoord;

void main()
{
	vec4 viewPos = g_ViewMatrix * g_ModelMatrix * vec4(position, 1.0);
	frag_position = viewPos.xyz;
	gl_Position = g_ProjectionMatrix * viewPos;
	
	frag_texCoord = vec2(0.0, 0.0);
	
	mat3 normalMatrix = transpose(inverse(mat3(g_ViewMatrix * g_ModelMatrix)));
	frag_normal = normalize(normalMatrix * normal);
}