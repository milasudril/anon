.PHONY: all
all: dynlib staticlib

.PHONY: dynlib
dynlib:
	maike2 --configfiles=maikeconfig-base.json,maikeconfig-dynlib.json --target-dir=__targets_dynlib

.PHONY: staticlib
staticlib:
	maike2 --configfiles=maikeconfig-base.json --target-dir=__targets_staticlib

.PHONY: clean
clean:
	rm -rf __targets*

.PHONY: doc
doc:
	rm -rf __targets_doc
	doxygen

DESTDIR?=""
PREFIX?="/usr"
.PHONY: install
install: make_pkgconfig.sh
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/include/anon
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp __targets_staticlib/libanon.a $(DESTDIR)$(PREFIX)/lib/libanon.a
	find -maxdepth 1 -name '*.hpp' \
	    | while read in; do grep -v '^//@' "$$in" \
	    > $(DESTDIR)$(PREFIX)/include/anon/$$in; done
	mkdir -p $(DESTDIR)$(PREFIX)/lib/pkgconfig
	./make_pkgconfig.sh $(PREFIX) $(DESTDIR)$(PREFIX)/lib/pkgconfig/anon.pc
	mkdir -p $(DESTDIR)$(PREFIX)/lib/python3/dist-packages
	cp __targets_dynlib/anonpy.so $(DESTDIR)$(PREFIX)/lib/python3/dist-packages/
