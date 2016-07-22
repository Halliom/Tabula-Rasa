#include "ScriptEngine.h"

#include <string>
#include <exception>

#include "../Platform/Platform.h"
#include "../Game/World.h"
#include "../Game/Player.h"
#include "../Engine/Console.h"
#include "../Engine/Engine.h"
#include "../Engine/Noise.h"
#include "../Engine/Chunk.h"

void LogToConsole(std::string Message)
{
	g_Engine->g_Console->PrintLine(Message);
}

void WorldWrapper::AddBlock(int X, int Y, int Z, int BlockID)
{
	g_Engine->g_World->AddBlock(X, Y, Z, BlockID);
}

void WorldWrapper::RemoveBlock(int X, int Y, int Z)
{
	g_Engine->g_World->RemoveBlock(X, Y, Z);
}

void WorldWrapper::AddMultiblock(int X, int Y, int Z, int BlockID)
{
	g_Engine->g_World->AddMultiblock(X, Y, Z, BlockID);
}

void WorldWrapper::RemoveMultiblock(int X, int Y, int Z)
{
	g_Engine->g_World->RemoveMultiblock(X, Y, Z);
}

Player* WorldWrapper::GetPlayer()
{
	return g_Engine->g_World->m_pCurrentPlayer;
}

float SimplexNoiseWrapper::Noise(float X, float Y)
{
    return m_pNoise->Noise(X, Y);
}

void ChunkWrapper::SetBlock(int X, int Y, int Z, int BlockID)
{
    m_pChunk->SetVoxel(m_pWorldObject, X, Y, Z, BlockID, NULL);
}

LuaTable::LuaTable(lua_State* L) :
    L(L),
    m_Reference(0)
{
}

TableIterator LuaTable::GetIterator()
{
    return TableIterator(this, L);
}

FORCEINLINE void LuaTable::CreateNew()
{
    lua_newtable(L);
    BindToExisting();
}

FORCEINLINE void LuaTable::BindToExisting()
{
    // Create a reference to the object
    m_Reference = luaL_ref(L, LUA_REGISTRYINDEX);
}

void LuaTable::PushToStack()
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_Reference);
}

LuaValue LuaTable::GetValue(const char* Name)
{
    int Top = lua_gettop(L);
    // Push the table on to the stack
    PushToStack();
    
    // Pushes the key onto the stack
    lua_pushstring(L, Name);
    
    // Gets the vale of this table (which is at position -2) using the key
    // which is at position -1
    lua_gettable(L, -2);
    
    // Result is at the top of the stack, pop it into a variable
    LuaValue Result = g_Engine->g_ScriptEngine->PopValue();
    
    // Since we don't know how many things were pushed onto the stack
    // since the start of the function, just reset it to where it was
    // before the function was called, that way everything above that
    // will be popped
    lua_settop(L, Top);
    
    return Result;
}

TableIterator::TableIterator(LuaTable* Table, lua_State* L) :
    HasNext(true),
    L(L),
    m_pTable(Table),
    m_Key(LUA_TNIL),
    m_Value(LUA_TNIL)
{
    // Put the table on the stack
    m_pTable->PushToStack();
    
    // Get the key-value-pair of the first table element
    Next();
}

void TableIterator::Next()
{
    // Push the key onto the stack (first time it will be nil)
    g_Engine->g_ScriptEngine->PushValue(m_Key);
    
    // lua_next returns true if there is another index left
    HasNext = lua_next(L, -2);
    
    if (HasNext)
    {
        // Now the value is at the top (-1) and the key is at (-2)
        m_Value = g_Engine->g_ScriptEngine->PopValue();
        m_Key = g_Engine->g_ScriptEngine->PopValue();
    }
    else
    {
        // No new value just pop the key off the stack
        lua_pop(L, 1);
    }
}

ScriptEngine::ScriptEngine()
{
    // Create a new lua state which keeps basically everything
    L = luaL_newstate();
    
    // Initialize the lua standard library functions
    luaL_openlibs(L);
    
    luabridge::getGlobalNamespace(L)
        .addFunction("print", &LogToConsole)
        .beginClass<Player>("player")
            .addFunction("set_speed", &Player::SetMovementSpeed)
            .addFunction("set_position", &Player::SetPositionLua)
        .endClass()
        .beginNamespace("world")
            .addFunction("add_block", &WorldWrapper::AddBlock)
			.addFunction("remove_block", &WorldWrapper::RemoveBlock)
			.addFunction("add_multiblock", &WorldWrapper::AddMultiblock)
			.addFunction("remove_mutliblock", &WorldWrapper::RemoveMultiblock)
			.addFunction("get_player", &WorldWrapper::GetPlayer)
        .endNamespace()
        .beginClass<SimplexNoiseWrapper>("SimplexNoiseWrapper")
            .addFunction("noise", &SimplexNoiseWrapper::Noise)
        .endClass()
        .beginClass<ChunkWrapper>("ChunkWrapper")
            .addFunction("set_block", &ChunkWrapper::SetBlock)
        .endClass();
}

ScriptEngine::~ScriptEngine()
{
    lua_close(L);
}

std::string ScriptEngine::GetErrorMessage()
{
    // Error message is alwasy at the top of the stack, -1 means top of stack
    const char* ErrorMessage = lua_tostring(L, -1);
    
    // Create a copy since lua does garbage collection
    std::string Result = std::string(ErrorMessage);
    
    // Removes the message from the stack
    lua_pop(L, 1);
    
    return Result;
}

void ScriptEngine::ExecuteString(const std::string& Source, const char* FileName)
{
    int Status = luaL_loadbuffer(L, Source.c_str(), Source.size(), FileName);
    
    switch (Status)
    {
        case 0: // No errors
        {
            Status = lua_pcall(L, 0, LUA_MULTRET, 0);
            
            switch (Status)
            {
                case 0:
                {
                    // Everything is fine and the script is properly loaded
                    m_LoadedScripts.insert(std::string(FileName));
                    break;
                }
                case LUA_ERRRUN: // Runtime error
                {
                    std::string Error = GetErrorMessage();
                    LogError("Lua runtime error");
                    LogF("Error running script: %s\n%s", FileName, Error.c_str());
                    break;
                }
                case LUA_ERRMEM:
                {
                    LogError("Lua Memory allocation error");
                    LogF("Error running script: %s, memory error", FileName);
                }
            }
            
            break;
        }
        case LUA_ERRSYNTAX: // Syntax error
        {
            std::string Error = GetErrorMessage();
            LogError("Lua syntax error");
            LogF("Error compiling script: %s\n%s", FileName, Error.c_str());
            break;
        }
        case LUA_ERRMEM: // Allocation error, should not happen
        {
            LogError("Lua Memory allocation error");
            LogF("Error loading script: %s, memory error", FileName);
            break;
        }
    }
}

void ScriptEngine::ExecuteScript(const char* FileName)
{
    std::string ScriptSource = PlatformFileSystem::LoadFile(DT_SCRIPTS, FileName);
    
    ExecuteString(ScriptSource, FileName);
}

FORCEINLINE void ScriptEngine::ExecuteScript(const std::string& FileName)
{
    ExecuteScript(FileName.c_str());
}

void ScriptEngine::ExecuteInInterpreter(const char* String)
{
    ExecuteString(std::string(String));
}

void ScriptEngine::SetupFunction(const char* FunctionName)
{
    lua_getglobal(L, FunctionName);
    
    assert(lua_isboolean(L, -1));
}

void ScriptEngine::SetupFunction(LuaFunction FunctionReference)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, FunctionReference);
}

LuaValue ScriptEngine::CallFunction(int NumArgs)
{
    LuaValue Result;
    int Status = lua_pcall(L, NumArgs, 1, NULL);
    
    switch (Status)
    {
        case 0:
        {
            // Function call succeeded
            Result = PopValue();
            break;
        }
        case LUA_ERRRUN:
        {
            LogError("Error calling function:");
            std::string Error = GetErrorMessage();
            LogError(Error.c_str());
            break;
        }
        case LUA_ERRMEM:
        {
            // Should never happen
            assert(0);
            break;
        }
    }
    
    return Result;
}

void ScriptEngine::ReloadScript(const char* FileName)
{
    ExecuteScript(FileName);
}

void ScriptEngine::ReloadAllScripts()
{
    for (auto& Script : m_LoadedScripts)
    {
        ReloadScript(Script.c_str());
    }
}

bool ScriptEngine::GetBoolean(const char* Name)
{
    lua_getglobal(L, Name);
    
    assert(lua_isboolean(L, -1));
    
    bool Result = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    return Result;
}

int ScriptEngine::GetInt(const char* Name)
{
    lua_getglobal(L, Name);
    
    assert(lua_isnumber(L, -1));
    
    int Result = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    return Result;
}

float ScriptEngine::GetFloat(const char* Name)
{
    lua_getglobal(L, Name);
    
    assert(lua_isnumber(L, -1));
    
    float Result = lua_tonumber(L, -1);
    lua_pop(L, 1);
    
    return Result;
}

std::string ScriptEngine::GetString(const char* Name)
{
    lua_getglobal(L, Name);
    
    assert(lua_isstring(L, -1));
    
    std::string Result = std::string(lua_tostring(L, -1));
    lua_pop(L, 1);
    
    return Result;
}

LuaTable* ScriptEngine::GetTable(const char* Name)
{
    lua_getglobal(L, Name);
    
    assert(lua_istable(L, -1));
    
    LuaTable* Result = new LuaTable(L);
    Result->BindToExisting();
    
    return Result;
}

void ScriptEngine::SetBoolean(const char* Name, bool Value)
{
    lua_pushboolean(L, Value);
    
    lua_setglobal(L, Name);
}

void ScriptEngine::SetInt(const char* Name, int Value)
{
    lua_pushinteger(L, Value);
    
    lua_setglobal(L, Name);
}

void ScriptEngine::SetFloat(const char* Name, float Value)
{
    lua_pushnumber(L, Value);
    
    lua_setglobal(L, Name);
}

void ScriptEngine::SetString(const char* Name, const std::string& Value)
{
    lua_pushstring(L, Value.c_str());
    
    lua_setglobal(L, Name);
}

void ScriptEngine::SetTable(const char* Name, LuaTable* Value)
{
    Value->PushToStack();
    
    lua_setglobal(L, Name);
}

void ScriptEngine::PushValue(bool Value)
{
    lua_pushboolean(L, Value);
}

void ScriptEngine::PushValue(int Value)
{
    lua_pushnumber(L, Value);
}

void ScriptEngine::PushValue(float Value)
{
    lua_pushnumber(L, Value);
}

void ScriptEngine::PushValue(const std::string& Value)
{
    lua_pushstring(L, Value.c_str());
}

void ScriptEngine::PushValue(LuaTable* Value)
{
    Value->PushToStack();
}

void ScriptEngine::PushValue(const LuaValue& Value)
{
    switch(Value.m_ValueType)
    {
        case LUA_TNIL:
        {
            lua_pushnil(L);
            break;
        }
        case LUA_TNUMBER:
        {
            lua_pushnumber(L, Value.Number);
            break;
        }
        case LUA_TBOOLEAN:
        {
            lua_pushboolean(L, Value.Boolean);
            break;
        }
        case LUA_TSTRING:
        {
            lua_pushstring(L, Value.String);
            break;
        }
        case LUA_TTABLE:
        {
            Value.Table->PushToStack();
            break;
        }
        case LUA_TFUNCTION:
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, Value.Function);
            break;
        }
    }
}

LuaValue ScriptEngine::PopValue()
{
    LuaValue Result(lua_type(L, -1));
    switch (Result.m_ValueType)
    {
        case LUA_TNUMBER:
        {
            Result.Number = lua_tonumber(L, -1);
            lua_pop(L, 1);
            break;
        }
        case LUA_TBOOLEAN:
        {
            Result.Boolean = lua_toboolean(L, -1);
            lua_pop(L, 1);
            break;
        }
        case LUA_TSTRING:
        {
            Result.String = lua_tostring(L, -1);
            lua_pop(L, 1);
            break;
        }
        case LUA_TTABLE:
        {
            Result.Table = new LuaTable(L);
            Result.Table->BindToExisting();
            break;
        }
        case LUA_TFUNCTION:
        {
            Result.Function = luaL_ref(L, LUA_REGISTRYINDEX);
            break;
        }
    }
    return Result;
}
