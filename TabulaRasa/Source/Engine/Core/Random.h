#pragma once

#include <cstdint>

class Random
{
public:

	Random();

	Random(int Seed);

	Random(uint64_t Seed1, uint64_t Seed2);

	uint64_t Next();

	int NextInt();

	int NextInt(int Max);

	float NextFloat();

	uint64_t Seed[2];
};