#include "PythonScript.h"

#include "Console.h"

extern Console* g_Console;

PyObject* __cdecl Write(PyObject* self, PyObject* args)
{
	const char* Output;
	if (!PyArg_ParseTuple(args, "s", &Output))
		return NULL;
	g_Console->Print((char*)Output);

	return Py_BuildValue("");
}

PyObject* __cdecl Flush(PyObject* self, PyObject* args)
{
	return Py_BuildValue("");
}

PyObject* aview_write(PyObject* self, PyObject* args)
{
	return Write(self, args);
}

PyObject* aview_flush(PyObject* self, PyObject* args)
{
	return Flush(self, args);
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
	Py_Initialize();

	m_InterpreterContext = { 0 };
	m_InterpreterContext.Main = PyImport_ImportModule("__main__");
	m_InterpreterContext.Locals = PyModule_GetDict(m_InterpreterContext.Main);
	//PyObject_GetAttr();

	PyImport_AppendInittab("aview", PyInit_aview);
	PyImport_ImportModule("aview");
	PyRun_String("import aview", Py_single_input, m_InterpreterContext.Locals, m_InterpreterContext.Locals);
}

void PythonScriptEngine::Destroy()
{
	Py_Finalize();
}

void PythonScriptEngine::ExecuteStringInInterpreter(char* String)
{
	PyObject* Result = PyRun_String(String, Py_single_input, m_InterpreterContext.Locals, m_InterpreterContext.Locals);
	if (!Result)
	{

	}
	PyObject* ObjectRepr = PyObject_Repr(Result);
	PyObject* Unicode = PyUnicode_AsUTF8String(ObjectRepr);
	char* PythonResultString = PyBytes_AsString(Unicode);
	unsigned int StringLength = strlen(PythonResultString);
	char* ResultString = new char[StringLength];

	strcpy(ResultString, PythonResultString);
	Py_DecRef(Unicode);
	Py_DecRef(Result);
}

char* PythonScriptEngine::GetVariableValue(char* VariableName)
{
	PyObject* Result = PyDict_GetItemString(m_InterpreterContext.Locals, VariableName);
	if (!Result)
		return "";

	PyObject* ObjectRepr = PyObject_Repr(Result);
	PyObject* Unicode = PyUnicode_AsUTF8String(ObjectRepr);
	char* String = PyBytes_AsString(Unicode);
	unsigned int StringLength = strlen(String);
	char* ResultString = new char[StringLength];

	strcpy(ResultString, String);
	Py_DECREF(Unicode);
	return ResultString;
}

PythonScript PythonScriptEngine::CreateScript(char* ScriptName, char* ScriptString)
{
	PythonScript NewScript = { 0 };
	//PyParser_SimpleParseStringFlagsFilename
	PyObject* CompiledScript = Py_CompileString(ScriptString, NULL, Py_file_input);

	if (CompiledScript == NULL)
	{
		//TODO: Compilation error
	}
	NewScript.CompiledSource = CompiledScript;

	NewScript.Context.Main = PyImport_ImportModule("__main__");
	NewScript.Context.Locals = PyModule_GetDict(m_InterpreterContext.Main);
	PyRun_String("import aview", Py_single_input, NewScript.Context.Locals, NewScript.Context.Locals);

	return NewScript;
}