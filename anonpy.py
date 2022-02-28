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

if __name__ == '__main__':
	with object() as obj:
		pass

