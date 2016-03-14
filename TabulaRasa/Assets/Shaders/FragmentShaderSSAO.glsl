#version 330

// Only ouput a float since SSAO is grayscale
out float FragColor;

uniform sampler2D textureSampler0; // Position
uniform sampler2D textureSampler1; // Normal
uniform sampler2D textureSampler2; // SSAONoise

in vec2 frag_texCoord;

uniform mat4 g_ProjectionMatrix;
uniform vec3 g_Samples[64];

const vec2 noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0); // screen = 1280x720

const float radius = 1.0;

void main()
{
	// Get input for SSAO algorithm
    vec3 fragPos = texture(textureSampler0, frag_texCoord).xyz;
    vec3 normal = texture(textureSampler1, frag_texCoord).xyz;
    vec3 randomVec = texture(textureSampler2, frag_texCoord * noiseScale).xyz;
    // Create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // Iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < 64; ++i)
    {
        // get sample position
        vec3 sample = TBN * g_Samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = g_ProjectionMatrix * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = -texture(textureSampler0, offset.xy).w; // Get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / 64);
    
    FragColor = occlusion;
}