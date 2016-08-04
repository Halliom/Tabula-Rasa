#pragma once

#include <stdint.h>

#include "GLFW/glfw3.h"

#include <unordered_map>

enum EKey
{
	
};

enum EAxis
{
	EAxis_MouseY,
	EAxis_MouseX
};

class Input
{
public:
	static bool Keys[512];

	static bool MouseButtons[65];

	static int MouseX;
	static int MouseY;

	static float MouseWheel;

	static bool IsGameFrozen;
};

typedef void(*ActionCallbackFunction)();
typedef void(*AxisCallbackFunction)(float);

class InputManager
{
public:

	void RegisterActionCallback(EKey KeyToMap, ActionCallbackFunction Callback);

	void RegisterAxisCallback(EAxis AxisToMap, AxisCallbackFunction Callback);

	void ProcessInput(uint32_t KeyCode);

	//std::unordered_map<EKey, ActionCallbackFunction> m_ActionMappings;

	//std::unordered_map<EAxis, AxisCallbackFunction> m_AxisMappings;
};