#version 330
// Basic pass-through shader
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 textureCoord;

out vec2 frag_texCoord;

void main()
{
	gl_Position = position;
	frag_texCoord = textureCoord;
}