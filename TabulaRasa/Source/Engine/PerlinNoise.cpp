#include "PerlinNoise.h"

#include <random>
#include <math.h>

float lerp(float a0, float a1, float w) 
{
	return (1.0 - w)*a0 + w*a1;
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(float Gradient[16][16][2], int ix, int iy, float x, float y) 
{
	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx * Gradient[iy][ix][0] + dy * Gradient[iy][ix][1]);
}

float perlin(float Gradient[16][16][2], float x, float y) 
{
	// Determine grid cell coordinates
	int x0 = (x > 0.0 ? (int) x - 1 : (int) x);
	int x1 = x0 + 1;
	int y0 = (y > 0.0 ? (int) y - 1 : (int) y);
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx = x - (float) x0;
	float sy = y - (float) y0;

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;
	n0 = dotGridGradient(Gradient, x0, y0, x, y);
	n1 = dotGridGradient(Gradient, x1, y0, x, y);
	ix0 = lerp(n0, n1, sx);
	n0 = dotGridGradient(Gradient, x0, y1, x, y);
	n1 = dotGridGradient(Gradient, x1, y1, x, y);
	ix1 = lerp(n0, n1, sx);
	value = lerp(ix0, ix1, sy);

	return value;
}

PerlinNoise::PerlinNoise()
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 16; ++j)
		{
			Gradient[i][j][0] = float(rand() % 100) / 100.0f;
			Gradient[i][j][1] = float(rand() % 100) / 100.0f;
		}
	}
}

int PerlinNoise::GetHeight(int x, int y)
{
	return 10.0f * sin(x * 0.1) * sin(y * 0.1);
}