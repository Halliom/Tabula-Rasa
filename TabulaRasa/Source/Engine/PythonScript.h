#pragma once

#ifdef _DEBUG
#undef _DEBUG //Prevent linking debug build of python
#include <Python.h>
#define _DEBUG 1
#endif

class PythonScriptEngine
{
public:

	void Initialize();

	void Destroy();

};
