#pragma once

#include "glm\common.hpp"
#include "../Engine/Console.h"

extern Console* g_Console;

#define LogLn(str) g_Console->PrintLine(str);
#define Log(str) g_Console->Print(str);

__forceinline bool IsWithinBounds(const glm::uvec3& Position, const glm::uvec3& Bounds) //TODO: SIMD optimize from custom vector class
{
	if ((Position.x < Bounds.x) && (Position.x >= 0) &&
		(Position.y < Bounds.y) && (Position.y >= 0) &&
		(Position.z < Bounds.z) && (Position.z >= 0))
		return true;
	else
		return false;
}

__forceinline bool IsContainedWithin(const glm::uvec3& Position, const glm::uvec3& Min, const glm::uvec3& Max) //TODO: SIMD optimize from custom vector class
{
	if ((Position.x < Max.x) && (Position.x >= Min.x) &&
		(Position.y < Max.y) && (Position.y >= Min.y) &&
		(Position.z < Max.z) && (Position.z >= Min.z))
		return true;
	else
		return false;
}