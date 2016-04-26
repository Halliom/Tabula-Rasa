#pragma once

#include "lua5.2\lua.hpp"
#include "LuaBridge/LuaBridge.h"

struct WorldWrapper
{
	static void AddBlock(int X, int Y, int Z, int BlockID);
	static void RemoveBlock(int X, int Y, int Z);
	static void AddMultiblock(int X, int Y, int Z, int BlockID);
	static void RemoveMultiblock(int X, int Y, int Z);
};

class Script
{
public:

	Script(char* Filename);

	void CallMethod(char* MethodName);

	void LogErrors();

	char* m_pScriptName;

	static lua_State* g_State;
};