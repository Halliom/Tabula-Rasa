#include "ScriptEngine.h"

#include <string>
#include <exception>

#include "Console.h"
#include "../Game/World.h"
#include "../Platform/Platform.h"

extern Console* g_Console;
extern World* g_World;

lua_State* Script::g_State = NULL;

void LogToConsole(std::string Message)
{
	g_Console->PrintLine(Message);
}

void WorldWrapper::AddBlock(int X, int Y, int Z, int BlockID)
{
	g_World->AddBlock(X, Y, Z, BlockID);
}

void WorldWrapper::RemoveBlock(int X, int Y, int Z)
{
	g_World->RemoveBlock(X, Y, Z);
}

void WorldWrapper::AddMultiblock(int X, int Y, int Z, int BlockID)
{
	g_World->AddMultiblock(X, Y, Z, BlockID);
}

void WorldWrapper::RemoveMultiblock(int X, int Y, int Z)
{
	g_World->RemoveMultiblock(X, Y, Z);
}

Script::Script(char* Filename)
{
	m_pScriptName = Filename;
	char* ScriptSource = PlatformFileSystem::LoadScript(Filename);
	
	if (!g_State)
	{
		g_State = luaL_newstate();
		luaL_openlibs(g_State);

		luabridge::getGlobalNamespace(g_State)
			.addFunction("print", &LogToConsole)
			.beginNamespace("world")
				.addFunction("add_block", &WorldWrapper::AddBlock)
				.addFunction("remove_block", &WorldWrapper::RemoveBlock)
				.addFunction("add_multiblock", &WorldWrapper::AddMultiblock)
				.addFunction("remove_mutliblock", &WorldWrapper::RemoveMultiblock)
			.endNamespace();
	}

	int Error = luaL_dostring(g_State, ScriptSource);

	if (Error != 0)
	{
		LogF("Failed to load script: %s", Filename);
	}
}

void Script::CallFunction(char* FunctionName)
{
	try
	{
		luabridge::LuaRef Method = luabridge::getGlobal(g_State, FunctionName);
		Method();
	}
	catch (std::exception e)
	{
		LogFunctionErrors(FunctionName);
	}
}

bool Script::GetBool(char* VariableName)
{
	bool Result = false;
	lua_getglobal(g_State, VariableName);

	if (lua_isboolean(g_State, -1))
	{
		Result = lua_toboolean(g_State, -1);

		lua_pop(g_State, 1);
	}
	else
	{
		LogF("Error getting boolean %s, variable not boolean", VariableName);
	}
	return Result;
}

bool Script::GetBoolFromTable(char* TableName, char* VariableName)
{
	bool Result = 0;

	// Push the variable called TableName on top of the stack
	lua_getglobal(g_State, TableName);
	if (lua_istable(g_State, -1)) // If the value on top of the stack is a table
	{
		// Now the VariableName is on top of the stack
		lua_pushstring(g_State, VariableName);

		// Get the value of table (at stack pos -2) with index value of stack pos -1
		lua_gettable(g_State, -2);

		// Now the result from gettable is at the top of the stack
		if (lua_isboolean(g_State, -1))
		{
			Result = lua_toboolean(g_State, -1);
		}
		else
		{
			LogF("Error getting variable from table %s: variable is not a boolean", TableName);
		}

		// Since lua pops VariableName once gettable returns, there are now two
		// references on the stack, the result and the table
		lua_pop(g_State, 2);
	}
	else
	{
		LogF("Error getting variable %s from table %s", VariableName, TableName);
	}

	return Result;
}

int Script::GetInt(char* VariableName)
{
	try
	{
		luabridge::LuaRef Variable = luabridge::getGlobal(g_State, VariableName);
		if (Variable.isNumber())
		{
			return Variable.cast<int>();
		}
	}
	catch (std::exception e)
	{
		// Do nothing let it be handled by the next lines
	}

	// Something went wrong
	LogF("Error getting variable %s", VariableName);
	return 0;
}

int Script::GetIntFromTable(char* TableName, char* VariableName)
{
	int Result = 0;

	// Push the variable called TableName on top of the stack
	lua_getglobal(g_State, TableName);
	if (lua_istable(g_State, -1)) // If the value on top of the stack is a table
	{
		// Now the VariableName is on top of the stack
		lua_pushstring(g_State, VariableName);

		// Get the value of table (at stack pos -2) with index value of stack pos -1
		lua_gettable(g_State, -2);

		// Now the result from gettable is at the top of the stack
		if (lua_isnumber(g_State, -1))
		{
			Result = lua_tonumber(g_State, -1);
		}
		else
		{
			Log("Error getting variable from table: variable is not a number");
		}

		// Since lua pops VariableName once gettable returns, there are now two
		// references on the stack, the result and the table
		lua_pop(g_State, 2);
	}
	else
	{
		Log("Error getting variable from table");
	}

	return Result;
}

std::string Script::GetString(char* VariableName)
{
	try
	{
		luabridge::LuaRef Variable = luabridge::getGlobal(g_State, VariableName);
		if (Variable.isString())
		{
			return Variable.cast<std::string>();
		}
	}
	catch (std::exception e)
	{
		// Do nothing let it be handled by the next lines
	}

	Log("Error getting variable");
	return std::string("");
}

std::string Script::GetStringFromTable(char* TableName, char* VariableName)
{
	std::string Result = "";

	// Push the variable called TableName on top of the stack
	lua_getglobal(g_State, TableName);
	if (lua_istable(g_State, -1)) // If the value on top of the stack is a table
	{
		// Now the VariableName is on top of the stack
		lua_pushstring(g_State, VariableName);

		// Get the value of table (at stack pos -2) with index value of stack pos -1
		lua_gettable(g_State, -2);

		// Now the result from gettable is at the top of the stack
		if (lua_isstring(g_State, -1))
		{
			Result = std::string(lua_tostring(g_State, -1));
		}
		else
		{
			Log("Error getting variable from table: variable is not a string");
		}

		// Since lua pops VariableName once gettable returns, there are now two
		// references on the stack, the result and the table
		lua_pop(g_State, 2);
	}
	else
	{
		Log("Error getting variable from table");
	}
	return Result;
}

void Script::LogFunctionErrors(char* FunctionName)
{
	char Buffer[300];
	sprintf(Buffer, "Error in script: \"%s\" while calling function: \"%s\":", m_pScriptName, FunctionName);

	LogToConsole(Buffer);
	LogToConsole(lua_tostring(g_State, -1));
	lua_pop(g_State, 1); // remove error message
}