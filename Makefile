CC=gcc
#CFLAGS=-Wall -Wstrict-prototypes -Werror -fPIC -DDEBUG -g -O0
CFLAGS=-Wall -Wstrict-prototypes -Werror -fPIC
LIBRARY=libnss_cache.so.2.0
LINKS=libnss_cache.so.2 libnss_cache.so
DESTDIR=/
PREFIX=$(DESTDIR)/usr
BUILD=.libs
BUILD64=.libs64
BUILD32=.libs32
LIBDIR=$(PREFIX)/lib
LIBDIR64=$(PREFIX)/lib64
LIBDIR32=$(PREFIX)/lib32
SONAME=libnss_cache.so.2
LD_SONAME=-Wl,-soname,$(SONAME)
TESTBIN=.testbin
TESTDATA=.testdata

SOURCES = Makefile gen_getent.c lookup.c nss_cache.c nss_cache.h nss_test.h COPYING
VERSION = $(shell cat version)

default: build_amd64
	echo N.B.  Defaulted to build_amd64!

test: test_getent time_lookups

time_lookups: lookup_data lookup.c nss_cache.c
	$(CC) $(CFLAGS) -o $(TESTBIN)/lookup lookup.c nss_cache.c
	@echo Linear username lookups
	rm -f $(TESTDATA)/passwd.cache.ixname
	time -f %E $(TESTBIN)/lookup -c getpwnam -f $(TESTDATA)/rand_pwnames
	@echo Binary username lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/passwd.cache 1 $(TESTDATA)/passwd.cache.ixname
	time -f %E $(TESTBIN)/lookup -c getpwnam -f $(TESTDATA)/rand_pwnames
	@echo Linear UID lookups
	rm -f $(TESTDATA)/passwd.cache.ixuid
	time -f %E $(TESTBIN)/lookup -c getpwuid -f $(TESTDATA)/rand_pwuids
	@echo Binary UID lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/passwd.cache 3 $(TESTDATA)/passwd.cache.ixuid
	time -f %E $(TESTBIN)/lookup -c getpwuid -f $(TESTDATA)/rand_pwuids
	@echo Linear groupname lookups
	rm -f $(TESTDATA)/group.cache.ixname
	time -f %E $(TESTBIN)/lookup -c getgrnam -f $(TESTDATA)/rand_grnames
	@echo Binary groupname lookups
	sort -t: -k1,1 $(TESTDATA)/group.cache > $(TESTDATA)/group.cache.ixname
	../vendetta/files/gen_cache.py $(TESTDATA)/group.cache 1 $(TESTDATA)/group.cache.ixname
	time -f %E $(TESTBIN)/lookup -c getgrnam -f $(TESTDATA)/rand_grnames
	@echo Linear GID lookups
	rm -f $(TESTDATA)/group.cache.ixgid
	time -f %E $(TESTBIN)/lookup -c getgrgid -f $(TESTDATA)/rand_grgids
	@echo Binary GID lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/group.cache 3 $(TESTDATA)/group.cache.ixgid
	time -f %E $(TESTBIN)/lookup -c getgrgid -f $(TESTDATA)/rand_grgids
	@echo Linear shadow lookups
	rm -f $(TESTDATA)/shadow.cache.ixname
	time -f %E $(TESTBIN)/lookup -c getspnam -f $(TESTDATA)/rand_spnames
	@echo Binary shadow lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/shadow.cache 1 $(TESTDATA)/shadow.cache.ixname
	time -f %E $(TESTBIN)/lookup -c getspnam -f $(TESTDATA)/rand_spnames

test_getent: getent_data gen_getent.c nss_cache.c
	$(CC) -o $(TESTBIN)/gen_getent gen_getent.c nss_cache.c
	sudo ./$(TESTBIN)/gen_getent
	diff $(TESTDATA)/passwd.cache $(TESTDATA)/passwd.cache.out
	diff $(TESTDATA)/group.cache $(TESTDATA)/group.cache.out
	diff $(TESTDATA)/shadow.cache $(TESTDATA)/shadow.cache.out

lookup_data: getent_data
	cut -d : -f 1 $(TESTDATA)/passwd.cache |\
	  sort -R | head -500 > $(TESTDATA)/rand_pwnames
	cut -d : -f 3 $(TESTDATA)/passwd.cache |\
	  sort -R | head -500 > $(TESTDATA)/rand_pwuids
	cut -d : -f 1 $(TESTDATA)/group.cache |\
	  sort -R | head -500 > $(TESTDATA)/rand_grnames
	cut -d : -f 3 $(TESTDATA)/group.cache |\
	  sort -R | head -500 > $(TESTDATA)/rand_grgids
	cut -d : -f 1 $(TESTDATA)/shadow.cache |\
	  sort -R | head -500 > $(TESTDATA)/rand_spnames

getent_data: testdirs
	getent passwd > $(TESTDATA)/passwd.cache
	getent group > $(TESTDATA)/group.cache
	sudo getent shadow > $(TESTDATA)/shadow.cache

testdirs:
	mkdir -p $(TESTDATA)
	mkdir -p $(TESTBIN)

build_i386: nss_cache64

build_amd64: nss_cache32

install_i386: install64

install_amd64: install32

nss_cache:
	[ -d $(BUILD) ] || mkdir $(BUILD)
	$(CC) $(CFLAGS) -c nss_cache.c -o $(BUILD)/nss_cache.o
	$(CC) -shared $(LD_SONAME) -o $(BUILD)/$(LIBRARY) $(BUILD)/nss_cache.o

nss_cache64: nss_cache
	[ -d $(BUILD64) ] || mkdir $(BUILD64)
	$(CC) $(CFLAGS) -m64 -c nss_cache.c -o $(BUILD64)/nss_cache.o
	$(CC) -m64 -shared $(LD_SONAME) -o $(BUILD64)/$(LIBRARY) $(BUILD64)/nss_cache.o

nss_cache32: nss_cache
	[ -d $(BUILD32) ] || mkdir $(BUILD32)
	$(CC) $(CFLAGS) -m32 -c nss_cache.c -o $(BUILD32)/nss_cache.o
	$(CC) -m32 -shared $(LD_SONAME) -o $(BUILD32)/$(LIBRARY) $(BUILD32)/nss_cache.o

clean:
	rm -rf $(BUILD)
	rm -rf $(BUILD32)
	rm -rf $(BUILD64)
	rm -rf $(TESTDATA)
	rm -rf $(TESTBIN)

install:
	[ -d $(LIBDIR) ] || install -d $(LIBDIR)
	install $(BUILD)/$(LIBRARY) $(LIBDIR)
	cd $(LIBDIR); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done

install64: install
	[ -d $(LIBDIR64) ] || install -d $(LIBDIR64)
	install $(BUILD64)/$(LIBRARY) $(LIBDIR64)
	cd $(LIBDIR64); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done

install32: install
	[ -d $(LIBDIR32) ] || install -d $(LIBDIR32)
	install $(BUILD32)/$(LIBRARY) $(LIBDIR32)
	cd $(LIBDIR32); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done

distclean: clean
	rm -f *~ \#*

dist:
	rm -rf libnss-cache-$(VERSION) libnss-cache-$(VERSION).tar libnss-cache-$(VERSION).tar.gz
	mkdir libnss-cache-$(VERSION)
	cp $(SOURCES) libnss-cache-$(VERSION)
	tar cf libnss-cache-$(VERSION).tar libnss-cache-$(VERSION)
	gzip -9 libnss-cache-$(VERSION).tar
	rm -rf libnss-cache-$(VERSION)

.PHONY: dist clean install install64 install32 distclean
