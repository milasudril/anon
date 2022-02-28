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
	anon::object::mapped_type to_int(PyObject& obj)
	{
		int overflow{};
		auto val = PyLong_AsLongLongAndOverflow(&obj, &overflow);
		if(overflow != 0)
		{
			throw std::out_of_range{"Integer value out of range"};
		}

		if(val >= static_cast<long long>(std::numeric_limits<int32_t>::min())
			&& val <= static_cast<long long>(std::numeric_limits<int32_t>::max()))
		{
			return anon::object::mapped_type{static_cast<int32_t>(val)};
		}
		return anon::object::mapped_type{static_cast<int64_t>(val)};
	}

	anon::object::mapped_type to_value(PyObject& obj)
	{
		auto const type = std::string_view{Py_TYPE(&obj)->tp_name};
		if(type == "int")
		{
			return to_int(obj);
		}
		throw std::runtime_error{std::string{"Unsupported type '"}.append(type).append("'")};
	}

	template<class T>
	PyObject* from_value_impl(T const&)
	{ throw std::runtime_error{"Unimplemented"};}

	PyObject* from_value_impl(int32_t x)
	{
		return PyLong_FromLong(x);
	}

	PyObject* from_value(anon::object::mapped_type const& val)
	{
		return std::visit([](auto const& item){
			return from_value_impl(item);
		}, val);
	}

	anon::object* get_pointer(PyObject* args)
	{
		PyObject* val{};
		if(!PyArg_ParseTuple(args, "O", &val))
		{ return nullptr; }

		return reinterpret_cast<anon::object*>(PyLong_AsVoidPtr(val));
	}

	PyObject* object_create(PyObject*, PyObject*)
	{
		auto obj = new anon::object{};
		return PyLong_FromVoidPtr(obj);
	}

	PyObject* object_contains(PyObject*, PyObject* args)
	{
		PyObject* obj{};
		char const* key{};
		if(!PyArg_ParseTuple(args, "Os", &obj, &key))
		{ return nullptr; }

		auto self = reinterpret_cast<anon::object*>(PyLong_AsVoidPtr(obj));
		return self->contains(key) ? Py_True : Py_False;
	}

	PyObject* object_insert_or_assign(PyObject*, PyObject* args)
	{
		try
		{
			PyObject* obj{};
			char const* key{};
			PyObject* val{};
			if(!PyArg_ParseTuple(args, "OsO", &obj, &key, &val))
			{ return nullptr; }

			auto self = reinterpret_cast<anon::object*>(PyLong_AsVoidPtr(obj));
			self->insert_or_assign(anon::key{key}, to_value(*val));
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* object_operator_brackets(PyObject*, PyObject* args)
	{
		try
		{
			PyObject* obj{};
			char const* key{};
			if(!PyArg_ParseTuple(args, "Os", &obj, &key))
			{ return nullptr; }

			auto self = reinterpret_cast<anon::object*>(PyLong_AsVoidPtr(obj));
			auto& val = (*self)[key];
			return from_value(val);
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	PyObject* object_size(PyObject*, PyObject* args)
	{
		return PyLong_FromSize_t(std::size(*get_pointer(args)));
	}

	PyObject* object_destroy(PyObject*, PyObject* args)
	{
		delete get_pointer(args);
		return Py_None;
	}

	constinit std::array<PyMethodDef, 7> method_table
	{
		PyMethodDef{"object_create", object_create, METH_VARARGS, ""},
		PyMethodDef{"object_destroy", object_destroy, METH_VARARGS, ""},
		PyMethodDef{"object_contains", object_contains, METH_VARARGS, ""},
		PyMethodDef{"object_insert_or_assign", object_insert_or_assign, METH_VARARGS, ""},
		PyMethodDef{"object_operator_brackets", object_operator_brackets, METH_VARARGS, ""},
		PyMethodDef{"object_size", object_size, METH_VARARGS, ""},


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