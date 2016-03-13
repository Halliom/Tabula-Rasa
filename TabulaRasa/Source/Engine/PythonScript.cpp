#include "PythonScript.h"

void PythonScriptEngine::Initialize()
{
	char string[1024];
	freopen("/dev/null", "a", stdout);
	setbuf(stdout, string);

	Py_Initialize();

	PyRun_SimpleString("a = 42 * 4");
	PyRun_SimpleString("a");

	PyObject* MainModule = PyImport_AddModule("__main__");

	PyObject* ModuleDictionary = PyModule_GetDict(MainModule);

	PyObject* result = PyDict_GetItemString(ModuleDictionary, "a");

	long result_value = PyLong_AsLong(result);
}

void PythonScriptEngine::Destroy()
{
	Py_Finalize();
}
