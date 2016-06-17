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

	static bool ExecuteStringInInterpreter(const char* InputString);

	void CallFunction(char* FunctionName);

	bool GetBool(char* VariableName);

	bool GetBoolFromTable(char* TableName, char* VariableName);

	int GetInt(char* VariableName);

	int GetIntFromTable(char* TableName, char* VariableName);

	std::string GetString(char* VariableName);

	std::string GetStringFromTable(char* TableName, char* VariableName);

	luabridge::LuaRef GetReference(const char* VariableName)
	{
		return luabridge::getGlobal(g_State, VariableName);
	}

	void LogFunctionErrors(char* FunctionName);

	char* m_pScriptName;

	static lua_State* g_State;
};