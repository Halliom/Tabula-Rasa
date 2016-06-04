#pragma once

#include "glm\common.hpp"

class SimplexNoise
{
public:
	
	SimplexNoise();

	float DotGridGradient(int ix, int iy, float x, float y);

	float Noise(float x, float y);

private:

	float dot(glm::ivec3 g, float x, float y)
	{
		return g[0] * x + g[1] * y;
	}

	float				m_Gradient[16][16][2];
	int					m_Permutations[512];

	static int			p[];
	static glm::ivec3	grad3[];

};
