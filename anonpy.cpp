//@	{
//@	 "target":{"name":"anonpy.o", "pkgconfig_libs":["python3"]}
//@	,"dependencies":[{"ref":"python3", "origin":"pkg-config"}]
//@	}

#define PY_SSIZE_T_CLEAN  /* Make "s#" use Py_ssize_t rather than int. */
#include <Python.h>

#include "./object.hpp"
#include "./deserializer.hpp"

#include <array>

namespace
{
	template<class T>
	PyObject* to_py_obj(T) = delete;

	PyObject* to_py_obj(anon::object const& obj);

	template<class T>
	PyObject* to_py_obj(std::vector<T> const& obj);

	PyObject* to_py_obj(std::string const& str)
	{
		return PyUnicode_FromString(str.c_str());
	}

	PyObject* to_py_obj(double val)
	{
		return PyFloat_FromDouble(val);
	}

	PyObject* to_py_obj(float val)
	{
		return PyFloat_FromDouble(val);
	}

	PyObject* to_py_obj(uint64_t val)
	{
		return PyLong_FromUnsignedLongLong(val);
	}

	PyObject* to_py_obj(int64_t val)
	{
		return PyLong_FromLongLong(val);
	}

	PyObject* to_py_obj(int32_t val)
	{
		return PyLong_FromLong(val);
	}

	PyObject* to_py_obj(uint32_t val)
	{
		return PyLong_FromUnsignedLong(val);
	}

	PyObject* to_py_obj(anon::object::mapped_type const& val)
	{
		return std::visit([]<class T>(T const& item) {
			return to_py_obj(item);
		}, val);
	}

	template<class T>
	PyObject* to_py_obj(std::vector<T> const& obj)
	{
		auto ret = PyList_New(std::size(obj));
		std::ranges::for_each(obj, [ret, k = static_cast<size_t>(0)](auto const& item) mutable {
			PyList_SetItem(ret, k, to_py_obj(item));
			++k;
		});
		return ret;
	}

	PyObject* to_py_obj(anon::object const& obj)
	{
		auto ret = PyDict_New();
		std::ranges::for_each(obj, [ret](auto const& item) {
			PyDict_SetItemString(ret, item.first.c_str(), to_py_obj(item.second));
		});
		return ret;
	}

	PyObject* load_from_path(PyObject*, PyObject* args)
	{
		try
		{
			char const* src_file{};
			if(!PyArg_ParseTuple(args, "s", &src_file))
			{ return nullptr; }

			return to_py_obj(anon::load(std::filesystem::path{src_file}));
		}
		catch(std::exception const& err)
		{
			PyErr_SetString(PyExc_RuntimeError, err.what());
			return nullptr;
		}

		return Py_None;
	}

	constinit std::array<PyMethodDef, 2> method_table
	{
		PyMethodDef{"load_from_path", load_from_path, METH_VARARGS, ""},
		PyMethodDef{nullptr, nullptr, 0, nullptr}
	};

	constinit PyModuleDef module_info
	{
		PyModuleDef_HEAD_INIT,
		"anonpy",
		nullptr,
		-1,
		std::data(method_table),
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
}

PyMODINIT_FUNC PyInit_anonpy()
{
	return PyModule_Create(&module_info);
}