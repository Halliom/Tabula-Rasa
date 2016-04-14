#version 330
uniform sampler2D textureSampler0;

in vec3 frag_color;
in vec2 frag_texCoord;

uniform vec4 g_Color;

out vec4 fragment_color;

const float font_smoothing = 1.0 / 16.0;
const vec4 TEXT_COLOR = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
	/*float dist = texture2D(textureSampler0, frag_texCoord).r;
	float alpha = smoothstep(0.5 - font_smoothing, 0.5 + font_smoothing, dist) * TEXT_COLOR.a;*/
	
	//fragment_color = vec4(texture2D(textureSampler0, frag_texCoord).r, 0.0, 0.0, 1.0);
	fragment_color = vec4(texture2D(textureSampler0, frag_texCoord).r * g_Color);
}