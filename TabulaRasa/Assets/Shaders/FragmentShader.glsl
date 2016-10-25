#version 330

//layout(location = 0) out vec4 OutPosition;
//layout(location = 1) out vec3 OutNormal;
//layout(location = 2) out vec3 OutTexCoord;

out vec4 OutColor;

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texCoord;
in vec2 frag_atlasOffset;

uniform sampler2D textureSampler0;

// Constants, should not be constant, should be set by the engine
const float NEAR = 0.1;
const float FAR = 160.0;

float linearizeDepth(in float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

// Every texture is a 16th of the entire
const vec2 TILE_OFFSET = vec2(1.0 / 16.0, 1.0 / 16.0);

void main()
{
  vec2 texCoord = frag_atlasOffset + (fract(frag_texCoord) * TILE_OFFSET);

  if (fract(frag_texCoord.x) <= 0.02 || fract(frag_texCoord.x) >= 0.98 ||
      fract(frag_texCoord.y) <= 0.02 || fract(frag_texCoord.y) >= 0.98)
  {
    OutColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
  else
  {
    OutColor = texture(textureSampler0, texCoord);
  }
	//OutPosition.xyz = frag_position;
	//OutPosition.a = linearizeDepth(gl_FragCoord.z);

	//OutNormal = frag_normal;
	//OutTexCoord = vec3(texCoord, 0.0);
}
