#pragma once

#include <stdint.h>

#include "SDL2\SDL.h"

class Input
{
public:
	static bool Keys[256];

	static bool MouseButtons[65];

	static int MouseX;
	static int MouseY;
};