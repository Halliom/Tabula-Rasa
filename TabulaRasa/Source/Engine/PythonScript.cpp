#include "PythonScript.h"

void PythonScriptEngine::Initialize()
{
	Py_Initialize();
}

void PythonScriptEngine::Destroy()
{
	Py_Finalize();
}
