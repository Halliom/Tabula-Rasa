#version 330

// Only ouput a float since SSAO is grayscale
out float FragColor;

uniform sampler2D textureSampler0; // Previous SSAO pass

in vec2 frag_texCoord;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(textureSampler0, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(textureSampler0, frag_texCoord + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
}