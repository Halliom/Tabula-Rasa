in vec3 position;
//in vec3 in_color;

uniform mat4 g_ProjectionMatrix;
uniform mat4 g_ViewMatrix;
uniform mat4 g_WorldTransformMatrix;

out vec4 frag_color;

void main()
{
	gl_Position = g_ProjectionMatrix * g_ViewMatrix * g_WorldTransformMatrix * vec4(position, 1.0f);
	frag_color = vec4(1.0, 1.0, 1.0, 1.0);
}