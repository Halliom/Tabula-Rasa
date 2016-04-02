#pragma once

#ifdef _DEBUG
#undef _DEBUG //Prevent linking debug build of python
#include <Python.h>
#define _DEBUG 1
#endif

struct PythonContext
{
	PyObject* Main;
	PyObject* Locals;
};

struct PythonScript
{
	PythonContext Context;

	PyObject* CompiledSource;
};

struct PythonReference
{
	PythonReference(PyObject* Object) : Object(Object) { ReferenceCount = new int; *ReferenceCount = 1; }
	PythonReference(PyObject* Object, int* ReferenceCount) : Object(Object), ReferenceCount(ReferenceCount) { ++(*ReferenceCount); }

	~PythonReference() 
	{ 
		if (--(*ReferenceCount) == 0)
		{
			Py_DecRef(Object);
		}
	}

	PyObject* Object;

	int* ReferenceCount;
};

class PythonScriptEngine
{
public:

	void Initialize();

	void Destroy();

	void ExecuteStringInInterpreter(char* String);

	char* GetVariableValue(char* VariableName);

	PythonScript CreateScript(char* ScriptName, char* ScriptString);

	void ExecuteScript();
	
	PythonContext m_InterpreterContext;

};
