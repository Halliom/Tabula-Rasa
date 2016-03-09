#version 430
uniform sampler2D textureSampler0;

in vec3 frag_color;
in vec2 frag_texCoord;

out vec4 fragment_color;
const float font_smoothing = 1.0 / 16.0;
const float font_buffer = 0.1;

const vec4 TEXT_COLOR = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
	float dist = texture2D(textureSampler0, frag_texCoord).a;
	//float alpha = smoothstep(font_buffer - font_smoothing, font_buffer + font_smoothing, dist);
	float alpha = smoothstep(0.5 - font_smoothing, 0.5 + font_smoothing, dist) * TEXT_COLOR.a;
	fragment_color = vec4(TEXT_COLOR.rgb, alpha);
}