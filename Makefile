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

DESTDIR?=""
PREFIX?="/usr"
.PHONY: install
install: all make_pkgconfig.sh
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/include/anon
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib/pkgconfig
	cp __targets_staticlib/libanon.a $(DESTDIR)$(PREFIX)/lib/libanon.a
	find -maxdepth 1 -name '*.hpp' \
	    | while read in; do grep -v '^//@' "$$in" \
	    > $(DESTDIR)$(PREFIX)/include/anon/$$in; done
	./make_pkgconfig.sh $(PREFIX) $(DESTDIR)$(PREFIX)/lib/pkgconfig/anon.pc
	cp __targets_dynlib/anonpy.so $(DESTDIR)$(PREFIX)/python3/dist-packages/
