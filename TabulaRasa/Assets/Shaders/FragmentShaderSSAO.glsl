#version 330

// Only ouput a float since SSAO is grayscale
out float FragColor;

uniform sampler2D textureSampler0; // Position
uniform sampler2D textureSampler1; // Normal
uniform sampler2D textureSampler2; // SSAONoise

in vec2 frag_texCoord;

uniform mat4 g_ProjectionMatrix;
uniform vec3 g_Samples[64];

const vec2 noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0); // screen = 800x600

const float radius = 1.0;

void main()
{
	vec3 fragPos 	= texture(textureSampler0, frag_texCoord).xyz;
	vec3 normal		= texture(textureSampler1, frag_texCoord).rgb;
	vec3 randomVec 	= texture(textureSampler2, frag_texCoord * noiseScale).xyz;  
	
	vec3 tangent 	= normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent 	= cross(normal, tangent);
	mat3 TBN 		= mat3(tangent, bitangent, normal);
	
	float occlusion = 0.0;
	for (int i = 0; i < 64; ++i)
	{
		vec3 currentSample = TBN * g_Samples[i];
		currentSample = fragPos + currentSample * radius;
		
		vec4 offset = g_ProjectionMatrix * vec4(currentSample, 1.0);
		offset.xyz / offset.w; // view to clip-space
		offset.xyz = offset.xyz * 0.5f + 0.5f; // between 0 and 1
		
		float sampleDepth = -texture(textureSampler0, offset.xy).w; // depth in the alpha (w) component
		
		occlusion += (sampleDepth >= currentSample.z ? 1.0 : 0.0);
		
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= currentSample.z ? 1.0 : 0.0) * rangeCheck;    
	}
	
	FragColor = 1.0 - (occlusion / 64.0);
}