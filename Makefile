CC ?= gcc
CFLAGS ?= -Wall -Wstrict-prototypes
CFLAGS += -fPIC -pthread
LDFLAGS += -shared
LIBRARY=libnss_cache.so.2.0
BASE_LIBRARY=libnss_cache.so.2
PREFIX=$(DESTDIR)/usr
LIBDIR=$(PREFIX)/lib
SONAME=libnss_cache.so.2
LD_SONAME=-Wl,-soname,$(SONAME)
TESTDATA=.testdata

SOURCES = Makefile gen_getent.c lookup.c nss_cache.c nss_cache.h nss_test.h COPYING version libnss-cache.spec
VERSION = $(shell cat version)

all: $(LIBRARY)

check: test_getent time_lookups

lookup: lookup.o nss_cache.o
	$(CC) $(CFLAGS) -o $@ $^

time_lookups: testdirs lookup_data lookup
	@echo Linear username lookups
	rm -f $(TESTDATA)/passwd.cache.ixname
	time -f %E lookup -c getpwnam -f $(TESTDATA)/rand_pwnames
	@echo Binary username lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/passwd.cache 1 $(TESTDATA)/passwd.cache.ixname
	time -f %E lookup -c getpwnam -f $(TESTDATA)/rand_pwnames
	@echo Linear UID lookups
	rm -f $(TESTDATA)/passwd.cache.ixuid
	time -f %E lookup -c getpwuid -f $(TESTDATA)/rand_pwuids
	@echo Binary UID lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/passwd.cache 3 $(TESTDATA)/passwd.cache.ixuid
	time -f %E lookup -c getpwuid -f $(TESTDATA)/rand_pwuids
	@echo Linear groupname lookups
	rm -f $(TESTDATA)/group.cache.ixname
	time -f %E lookup -c getgrnam -f $(TESTDATA)/rand_grnames
	@echo Binary groupname lookups
	sort -t: -k1,1 $(TESTDATA)/group.cache > $(TESTDATA)/group.cache.ixname
	../vendetta/files/gen_cache.py $(TESTDATA)/group.cache 1 $(TESTDATA)/group.cache.ixname
	time -f %E lookup -c getgrnam -f $(TESTDATA)/rand_grnames
	@echo Linear GID lookups
	rm -f $(TESTDATA)/group.cache.ixgid
	time -f %E lookup -c getgrgid -f $(TESTDATA)/rand_grgids
	@echo Binary GID lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/group.cache 3 $(TESTDATA)/group.cache.ixgid
	time -f %E lookup -c getgrgid -f $(TESTDATA)/rand_grgids
	@echo Linear shadow lookups
	rm -f $(TESTDATA)/shadow.cache.ixname
	time -f %E lookup -c getspnam -f $(TESTDATA)/rand_spnames
	@echo Binary shadow lookups
	../vendetta/files/gen_cache.py $(TESTDATA)/shadow.cache 1 $(TESTDATA)/shadow.cache.ixname
	time -f %E lookup -c getspnam -f $(TESTDATA)/rand_spnames

gen_getent: gen_getent.o nss_cache.o
	$(CC) -o $@ $^


test_getent: getent_data gen_getent.c nss_cache.c
	sudo gen_getent
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

last_pw_errno_test: test/last_pw_errno_test.c
	$(CC) $(CFLAGS) -o $@ $^

testdirs:
	mkdir -p $(TESTDATA)

$(LIBRARY): nss_cache.o compat/getpwent_r.o compat/getgrent_r.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LD_SONAME) -o $(LIBRARY) $+

clean:
	rm -f $(LIBRARY) *.o lookup gen_getent last_pw_errno_test
	rm -rf $(TESTDATA)

install: all 
	install -d $(LIBDIR)
	install $(LIBRARY) $(LIBDIR)
	ln -sf $(LIBRARY) $(LIBDIR)/$(BASE_LIBRARY)

distclean: clean
	rm -f *~ \#*

dist:
	rm -rf libnss-cache-$(VERSION) libnss-cache-$(VERSION).tar libnss-cache-$(VERSION).tar.gz
	mkdir libnss-cache-$(VERSION)
	cp $(SOURCES) libnss-cache-$(VERSION)
	tar cf libnss-cache-$(VERSION).tar libnss-cache-$(VERSION)
	gzip -9 libnss-cache-$(VERSION).tar
	rm -rf libnss-cache-$(VERSION)

.PHONY: all dist clean install distclean
