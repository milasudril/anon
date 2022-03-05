.PHONY: all
all: dynlib staticlib

.PHONY: dynlib
dynlib:
	maike2 --configfiles=maikeconfig-dynlib.json --target-dir=__targets_dynlib

.PHONY: staticlib
staticlib:
	maike2 --configfiles=maikeconfig-staticlib.json --target-dir=__targets_staticlib

.PHONY: clean
clean:
	rm -rf __targets*