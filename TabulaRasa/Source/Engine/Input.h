#pragma once

#include <stdint.h>

class Input
{
public:
	static bool Keys[256];

	static bool MouseButtons[65];

	static uint32_t MouseX;
	static uint32_t MouseY;
};