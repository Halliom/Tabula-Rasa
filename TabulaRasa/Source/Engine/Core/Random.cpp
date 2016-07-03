#include "Random.h"

#ifdef _WIN32
#include "SDL2/SDL.h"
#elif __APPLE__
#include "SDL2OSX/SDL.h"
#endif

Random::Random()
{
	Seed[0] = SDL_GetTicks();
	Seed[1] = SDL_GetTicks() % 1216124;
}

Random::Random(int InSeed)
{
	Seed[0] = InSeed;
	Seed[1] = InSeed % 1216124;
}

Random::Random(uint64_t Seed1, uint64_t Seed2)
{
	Seed[0] = Seed1;
	Seed[1] = Seed2;
}

uint64_t Random::Next()
{
	uint64_t a = Seed[0];
	uint64_t b = Seed[1];

	Seed[0] = b;
	a ^= a << 23;
	a ^= a >> 18;
	a ^= b;
	a ^= b >> 5;
	Seed[1] = a;

	return a + b;
}

int Random::NextInt()
{
	return (int) Next();
}

int Random::NextInt(int Max)
{
	return NextInt() % Max;
}

float Random::NextFloat()
{
	return (float) NextInt(2147483648) / 2147483648.0f;
}
