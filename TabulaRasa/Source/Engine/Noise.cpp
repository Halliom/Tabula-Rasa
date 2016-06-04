#include "PerlinNoise.h"

#include <random>
#include <math.h>

int SimplexNoise::p[] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

glm::ivec3 SimplexNoise::grad3[] = { { 1,1,0 }, { -1,1,0 }, { 1,-1,0 }, { -1,-1,0 },
{ 1,0,1 },{ -1,0,1 },{ 1,0,-1 },{ -1,0,-1 },
{ 0,1,1 },{ 0,-1,1 },{ 0,1,-1 },{ 0,-1,-1 } };

SimplexNoise::SimplexNoise()
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			m_Gradient[i][j][0] = float(rand() % 100) / 100.0f;
			m_Gradient[i][j][1] = float(rand() % 100) / 100.0f;
		}
	}

	for (int i = 0; i < 512; ++i)
	{
		m_Permutations[i] = p[i & 255];
	}
}

float SimplexNoise::Noise(float x, float y)
{
	// Noise contributions from the three corners
	float n0, n1, n2; 

	float F2 = 0.5f * (sqrt(3.0f) - 1.0f);
	float s = (x + y) * F2; 
	int i = floor(x + s);
	int j = floor(y + s);

	float G2 = (3.0f - sqrt(3.0f)) / 6.0f;
	float t = (i + j) * G2;
	float X0 = i - t; // Unskew the cell origin back to (x,y) space
	float Y0 = j - t;
	float x0 = x - X0; // The x,y distances from the cell origin
	float y0 = y - Y0;
	
	int i1, j1; 
	
	if (x0 > y0) 
	{ 
		i1 = 1; 
		j1 = 0; 
	} 
	else 
	{ 
		i1 = 0; 
		j1 = 1; 
	}

	float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
	float y2 = y0 - 1.0 + 2.0 * G2;
	// Work out the hashed gradient indices of the three simplex corners
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = m_Permutations[ii + m_Permutations[jj]] % 12;
	int gi1 = m_Permutations[ii + i1 + m_Permutations[jj + j1]] % 12;
	int gi2 = m_Permutations[ii + 1 + m_Permutations[jj + 1]] % 12;
	// Calculate the contribution from the three corners
	float t0 = 0.5f - x0 * x0 - y0 * y0;
	if (t0 < 0)
	{
		n0 = 0.0;
	}
	else 
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
	}
	float t1 = 0.5f - x1 * x1 - y1 * y1;
	if (t1 < 0)
	{
		n1 = 0.0;
	}
	else 
	{
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	float t2 = 0.5f - x2 * x2 - y2 * y2;
	if (t2 < 0)
	{
		n2 = 0.0;
	}
	else 
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 70.0f * (n0 + n1 + n2);
}
