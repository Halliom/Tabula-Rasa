#pragma once

class PerlinNoise
{
public:

	PerlinNoise();

	int GetHeight(int x, int y);

private:

	float Gradient[16][16][2];

};