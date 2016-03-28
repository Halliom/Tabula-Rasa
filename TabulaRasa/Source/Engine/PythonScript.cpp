#include "PythonScript.h"

#include "Console.h"

static char string[1024];

extern Console* g_Console;

PyObject* aview_write(PyObject* self, PyObject* args)
{
	const char* Output;
	if (!PyArg_ParseTuple(args, "s", &Output))
		return NULL;
	g_Console->Print((char*)Output);

	return Py_BuildValue("");
}

PyObject* aview_flush(PyObject* self, PyObject* args)
{
	return Py_BuildValue("");
}

PyMethodDef aview_methods[] =
{
	{ "write", aview_write, METH_VARARGS, "doc for write" },
	{ "flush", aview_flush, METH_VARARGS, "doc for flush" },
	{ 0, 0, 0, 0 } // sentinel
};

PyModuleDef aview_module =
{
	PyModuleDef_HEAD_INIT, // PyModuleDef_Base m_base;
	"aview",               // const char* m_name;
	"doc for aview",       // const char* m_doc;
	-1,                    // Py_ssize_t m_size;
	aview_methods,        // PyMethodDef *m_methods
						  //  inquiry m_reload;  traverseproc m_traverse;  inquiry m_clear;  freefunc m_free;
};

PyMODINIT_FUNC PyInit_aview(void)
{
	PyObject* m = PyModule_Create(&aview_module);
	PySys_SetObject("stdout", m);
	PySys_SetObject("stderr", m);
	return m;
}

void PythonScriptEngine::Initialize()
{
	freopen(NULL, "a", stdout);
	setbuf(stdout, string);

	PyImport_AppendInittab("aview", PyInit_aview);
	Py_Initialize();
	PyImport_ImportModule("aview");

	PyRun_SimpleString("import aview");
}

void PythonScriptEngine::Destroy()
{
	Py_Finalize();
}

void PythonScriptEngine::ExecuteStringInInterpreter(char *String)
{
	PyRun_SimpleString(String);
}

char *PythonScriptEngine::GetVariableValue(char *VariableName)
{
	PyObject *MainModule = PyImport_AddModule("__main__");
	PyObject *ModuleDictionary = PyModule_GetDict(MainModule);

	PyObject *Result = PyDict_GetItemString(ModuleDictionary, VariableName);
	if (!Result)
		return "";
	
	PyObject *ObjectRepr = PyObject_Repr(Result);
	PyObject *Unicode = PyUnicode_AsUTF8String(ObjectRepr);
	char *String= PyBytes_AsString(Unicode);
	unsigned int StringLength = strlen(String);
	char *ResultString = new char[StringLength];

	strcpy(ResultString, String);
	Py_DECREF(Unicode);
	return ResultString;
}
