#version 430
uniform sampler2D textureSampler0;

in vec3 frag_color;
in vec2 frag_texCoord;

out vec4 fragment_color;
const float smoothing = 1.0 / 16.0;

void main()
{
	float dist = texture2D(textureSampler0, frag_texCoord).a;
	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, dist);
	fragment_color = vec4(alpha);
}