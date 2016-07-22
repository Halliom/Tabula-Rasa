#pragma once

#include <unordered_set>
#include <string>

#include "lua5.2/lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include "../Engine/Engine.h"

struct WorldWrapper
{
	static void AddBlock(int X, int Y, int Z, int BlockID);
	static void RemoveBlock(int X, int Y, int Z);
	static void AddMultiblock(int X, int Y, int Z, int BlockID);
	static void RemoveMultiblock(int X, int Y, int Z);
	static class Player* GetPlayer();
};

class SimplexNoiseWrapper
{
public:
    
    float Noise(float X, float Y);
    
    class SimplexNoise* m_pNoise;
};

class ChunkWrapper
{
public:
    
    void SetBlock(int X, int Y, int Z, int BlockID);
    
    class Chunk* m_pChunk;
    class World* m_pWorldObject;
};

// TODO: Maybe use something other than a list?
typedef std::unordered_set<std::string> ScriptSet;
typedef int LuaFunction;

class LuaTable
{
public:
    
    LuaTable(lua_State* L);
    
    class TableIterator GetIterator();
    
    void CreateNew();
    void BindToExisting();
    void PushToStack();
    
    class LuaValue GetValue(const char* Name);
    
private:
    
    lua_State*  L;
    int         m_Reference;
};

class LuaValue
{
public:
    LuaValue() :
        m_ValueType(LUA_TNIL)
    {}
    
    explicit LuaValue(int Type) :
        m_ValueType(Type)
    {}
    
    LuaValue(const LuaValue& Other)
    {
        memcpy(this, &Other, sizeof(LuaValue));
    }
    
    LuaValue& operator=(const LuaValue& Other)
    {
        memcpy(this, &Other, sizeof(LuaValue));
        
        return *this;
    }
    
    union
    {
        float           Number;
        bool            Boolean;
        const char*     String;
        LuaTable*       Table;
        void*           Pointer;
        LuaFunction     Function;
    };
    
    int     m_ValueType;
};

class TableIterator
{
public:
    
    TableIterator(LuaTable* Table, lua_State* L);
    
    TableIterator& operator++()
    {
        Next();
        return *this;
    }
    
    LuaValue& Key()
    {
        return m_Key;
    }
    
    LuaValue& Value()
    {
        return m_Value;
    }
    
    bool HasNext;
    
private:
    
    void Next();
    
    lua_State*  L;
    LuaTable*   m_pTable;
    LuaValue    m_Key;
    LuaValue    m_Value;
};

class ScriptEngine
{
    friend class Console;
public:
    
    ScriptEngine();
    
    ~ScriptEngine();
    
    void ExecuteScript(const char* FileName);
    void ExecuteScript(const std::string& FileName);
    void ExecuteInInterpreter(const char* String);
    
    // Sets up a function call, after this the arguments can be pushed
    void        SetupFunction(const char* FunctionName);
    void        SetupFunction(LuaFunction FunctionReference);
    
    // Calls the function (after arguments are pushed)
    LuaValue    CallFunction(int NumArgs);
    
    // Reloads all scripts or a specific one
    void        ReloadScript(const char* FileName);
    void        ReloadAllScripts();
    
    // Gets global values by name
    bool        GetBoolean(const char* Name);
    int         GetInt(const char* Name);
    float       GetFloat(const char* Name);
    std::string GetString(const char* Name);
    LuaTable*   GetTable(const char* Name);
    
    // Sets global values
    void        SetBoolean(const char* Name, bool Value);
    void        SetInt(const char* Name, int Value);
    void        SetFloat(const char* Name, float Value);
    void        SetString(const char* Name, const std::string& Value);
    void        SetTable(const char* Name, LuaTable* Value);
    
    // Pushes values to the top of the stack
    void        PushValue(bool Value);
    void        PushValue(int Value);
    void        PushValue(float Value);
    void        PushValue(const std::string& Value);
    void        PushValue(LuaTable* Value);
    void        PushValue(const LuaValue& Value);
    
    // Pops a LuaValue from the top of the stack
    LuaValue    PopValue();
    
    // TOOD: Make this private again
    lua_State*  L;
    
private:
    
    std::string GetErrorMessage();
    
    void ExecuteString(const std::string& Source, const char* FileName = "global");
    
    ScriptSet   m_LoadedScripts;
};

/* namespace Scripts
{
    LuaValue Call(const char* FunctionName)
    {
        g_Engine->g_ScriptEngine->SetupFunction(FunctionName);
        return g_Engine->g_ScriptEngine->CallFunction(0);
    }

    template<typename T1>
    LuaValue Call(const char* FunctionName, T1 Arg1)
    {
        g_Engine->g_ScriptEngine->SetupFunction(FunctionName);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        
        return g_Engine->g_ScriptEngine->CallFunction(1);
    }

    template<typename T1, typename T2>
    LuaValue Call(const char* FunctionName, T1 Arg1, T2 Arg2)
    {
        g_Engine->g_ScriptEngine->SetupFunction(FunctionName);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        
        return g_Engine->g_ScriptEngine->CallFunction(2);
    }

    template<typename T1, typename T2, typename T3>
    LuaValue Call(const char* FunctionName, T1 Arg1, T2 Arg2, T3 Arg3)
    {
        g_Engine->g_ScriptEngine->SetupFunction(FunctionName);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        g_Engine->g_ScriptEngine->PushValue(Arg3);
        
        return g_Engine->g_ScriptEngine->CallFunction(3);
    }

    template<typename T1, typename T2, typename T3, typename T4>
    LuaValue Call(const char* FunctionName, T1 Arg1, T2 Arg2, T3 Arg3, T4 Arg4)
    {
        g_Engine->g_ScriptEngine->SetupFunction(FunctionName);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        g_Engine->g_ScriptEngine->PushValue(Arg3);
        g_Engine->g_ScriptEngine->PushValue(Arg4);
        
        return g_Engine->g_ScriptEngine->CallFunction(4);
    }
    
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    LuaValue Call(const char* FunctionName, T1 Arg1, T2 Arg2, T3 Arg3, T4 Arg4, T5 Arg5)
    {
        g_Engine->g_ScriptEngine->SetupFunction(FunctionName);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        g_Engine->g_ScriptEngine->PushValue(Arg3);
        g_Engine->g_ScriptEngine->PushValue(Arg4);
        g_Engine->g_ScriptEngine->PushValue(Arg5);
        
        return g_Engine->g_ScriptEngine->CallFunction(5);
    }
    
    LuaValue Call(LuaFunction Function)
    {
        g_Engine->g_ScriptEngine->SetupFunction(Function);
        return g_Engine->g_ScriptEngine->CallFunction(0);
    }
    
    template<typename T1>
    LuaValue Call(LuaFunction Function, T1 Arg1)
    {
        g_Engine->g_ScriptEngine->SetupFunction(Function);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        
        return g_Engine->g_ScriptEngine->CallFunction(1);
    }
    
    template<typename T1, typename T2>
    LuaValue Call(LuaFunction Function, T1 Arg1, T2 Arg2)
    {
        g_Engine->g_ScriptEngine->SetupFunction(Function);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        
        return g_Engine->g_ScriptEngine->CallFunction(2);
    }
    
    template<typename T1, typename T2, typename T3>
    LuaValue Call(LuaFunction Function, T1 Arg1, T2 Arg2, T3 Arg3)
    {
        g_Engine->g_ScriptEngine->SetupFunction(Function);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        g_Engine->g_ScriptEngine->PushValue(Arg3);
        
        return g_Engine->g_ScriptEngine->CallFunction(3);
    }
    
    template<typename T1, typename T2, typename T3, typename T4>
    LuaValue Call(LuaFunction Function, T1 Arg1, T2 Arg2, T3 Arg3, T4 Arg4)
    {
        g_Engine->g_ScriptEngine->SetupFunction(Function);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        g_Engine->g_ScriptEngine->PushValue(Arg3);
        g_Engine->g_ScriptEngine->PushValue(Arg4);
        
        return g_Engine->g_ScriptEngine->CallFunction(4);
    }
    
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    LuaValue Call(LuaFunction Function, T1 Arg1, T2 Arg2, T3 Arg3, T4 Arg4, T5 Arg5)
    {
        g_Engine->g_ScriptEngine->SetupFunction(Function);
        
        g_Engine->g_ScriptEngine->PushValue(Arg1);
        g_Engine->g_ScriptEngine->PushValue(Arg2);
        g_Engine->g_ScriptEngine->PushValue(Arg3);
        g_Engine->g_ScriptEngine->PushValue(Arg4);
        g_Engine->g_ScriptEngine->PushValue(Arg5);
        
        return g_Engine->g_ScriptEngine->CallFunction(5);
    }
} */
