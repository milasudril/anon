#!/usr/bin/env python3

import sys
import sys
sys.path.append('__targets')
import anonpy

if __name__ == '__main__':
	obj = anonpy.load_from_path('./testdata/test.anon')
	print(obj)


