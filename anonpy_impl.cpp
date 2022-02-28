//@	{
//@	 "target":{"name":"anonpy_impl.o", "pkgconfig_libs":["python3"]}
//@	,"dependencies":[{"ref":"python3", "origin":"pkg-config"}]
//@	}

#define PY_SSIZE_T_CLEAN  /* Make "s#" use Py_ssize_t rather than int. */
#include <Python.h>

#include "./object.hpp"

#include <array>

namespace
{
	PyObject* object_create(PyObject*, PyObject*)
	{
		puts("Hej");
		auto obj = new anon::object{};
		return PyLong_FromVoidPtr(obj);
	}

	PyObject* object_destroy(PyObject*, PyObject* args)
	{
		puts("Då");
		PyObject* val{};
		if(!PyArg_ParseTuple(args, "O", &val))
		{ return nullptr; }

		auto ptr = reinterpret_cast<anon::object const*>(PyLong_AsVoidPtr(val));
		delete ptr;
		return Py_None;
	}

	constinit std::array<PyMethodDef, 3> method_table
	{
		PyMethodDef{"object_create", object_create, METH_VARARGS, "Creates an object"},
		PyMethodDef{"object_destroy", object_destroy, METH_VARARGS, "Destroys an object"},
		PyMethodDef{nullptr, nullptr, 0, nullptr}
	};

	constinit PyModuleDef module_info
	{
		PyModuleDef_HEAD_INIT,
		"anonpy_impl",
		nullptr,
		-1,
		std::data(method_table),
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
}

PyMODINIT_FUNC PyInit_anonpy_impl()
{
	return PyModule_Create(&module_info);
}