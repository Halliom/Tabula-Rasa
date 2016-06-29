#pragma once

#include "lua5.2/lua.hpp"
#include "LuaBridge/LuaBridge.h"

struct WorldWrapper
{
	static void AddBlock(int X, int Y, int Z, int BlockID);
	static void RemoveBlock(int X, int Y, int Z);
	static void AddMultiblock(int X, int Y, int Z, int BlockID);
	static void RemoveMultiblock(int X, int Y, int Z);
	static class Player* GetPlayer();
};

class Script
{
public:

	Script(const char* Filename);

	static bool ExecuteStringInInterpreter(const char* InputString);

	void CallFunction(const char* FunctionName);

	bool GetBool(const char* VariableName);

	bool GetBoolFromTable(const char* TableName, const char* VariableName);

	int GetInt(const char* VariableName);

	int GetIntFromTable(const char* TableName, const char* VariableName);

	std::string GetString(const char* VariableName);

	std::string GetStringFromTable(const char* TableName, const char* VariableName);

	luabridge::LuaRef GetReference(const char* VariableName)
	{
		return luabridge::getGlobal(g_State, VariableName);
	}

	void LogFunctionErrors(const char* FunctionName);

	const char* m_pScriptName;

	static lua_State* g_State;
};