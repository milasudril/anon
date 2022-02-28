#!/usr/bin/env python3

import sys
import sys
sys.path.append('__targets')
import anonpy_impl

class object:
	def __enter__(self):
		self.__handle = anonpy_impl.object_create()
		return self

	def __exit__(self, type, value, traceback):
		anonpy_impl.object_destroy(self.__handle)

	def contains(self, key):
		return anonpy_impl.object_contains(self.__handle, key)

	def insert_or_assign(self, key, value):
		anonpy_impl.object_insert_or_assign(self.__handle, key, value)
		return self

	def __getitem__(self, key):
		return anonpy_impl.object_operator_brackets(self.__handle, key)

	def size(self):
		return anonpy_impl.object_size(self.__handle)

if __name__ == '__main__':
	with object() as obj:
		assert(not obj.contains('foobar'))

		obj.insert_or_assign('foobar', 1)

		assert(obj.contains('foobar'))
		assert(obj['foobar'] == 1)
		assert(obj.size() == 1)


