#pragma once

#include "lua5.2\lua.hpp"
#include "LuaBridge/LuaBridge.h"

class Script
{
public:

	Script(char* Filename);

	void CallMethod(char* MethodName);

	static lua_State* g_State;
};