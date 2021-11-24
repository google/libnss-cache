CC ?= gcc
CFLAGS ?= -Wall -Wstrict-prototypes
CFLAGS += -fPIC
LIBRARY=libnss_cache.so
PREFIX=$(DESTDIR)/usr
LIBDIR=$(PREFIX)/lib
SONAME=libnss_cache.so.2
LD_SONAME=-Wl,-soname,$(SONAME)
TESTDATA=.testdata

LIBNSSCACHE = nss_cache.o compat/getpwent_r.o compat/getgrent_r.o

SOURCES = Makefile gen_getent.c lookup.c nss_cache.c nss_cache.h nss_test.h COPYING version libnss-cache.spec
VERSION = $(shell git describe --tags --always | sed -E 's@.*/([^-]*).*@\1@')

all: $(LIBRARY)

.PHONY: check
check: CFLAGS += -O0 -g --coverage
check: LDFLAGS += --coverage
check: test_getent time_lookups

lookup: lookup.o $(LIBNSSCACHE)

.PHONY: time_lookups
time_lookups: $(TESTDATA) lookup_data lookup
	@echo Linear username lookups
	rm -f $(TESTDATA)/passwd.cache.ixname
	time -f %E ./lookup -c getpwnam -f $(TESTDATA)/rand_pwnames
	@echo Binary username lookups
	./scripts/index.sh $(TESTDATA)/passwd.cache 1 $(TESTDATA)/passwd.cache.ixname
	time -f %E ./lookup -c getpwnam -f $(TESTDATA)/rand_pwnames

	@echo Linear UID lookups
	rm -f $(TESTDATA)/passwd.cache.ixuid
	time -f %E ./lookup -c getpwuid -f $(TESTDATA)/rand_pwuids
	@echo Binary UID lookups
	./scripts/index.sh $(TESTDATA)/passwd.cache 3 $(TESTDATA)/passwd.cache.ixuid
	time -f %E ./lookup -c getpwuid -f $(TESTDATA)/rand_pwuids

	@echo Linear groupname lookups
	rm -f $(TESTDATA)/group.cache.ixname
	time -f %E ./lookup -c getgrnam -f $(TESTDATA)/rand_grnames
	@echo Binary groupname lookups
	./scripts/index.sh $(TESTDATA)/group.cache 1 $(TESTDATA)/group.cache.ixname
	time -f %E ./lookup -c getgrnam -f $(TESTDATA)/rand_grnames

	@echo Linear GID lookups
	rm -f $(TESTDATA)/group.cache.ixgid
	time -f %E ./lookup -c getgrgid -f $(TESTDATA)/rand_grgids
	@echo Binary GID lookups
	./scripts/index.sh $(TESTDATA)/group.cache 3 $(TESTDATA)/group.cache.ixgid
	time -f %E ./lookup -c getgrgid -f $(TESTDATA)/rand_grgids

	@echo Linear shadow lookups
	rm -f $(TESTDATA)/shadow.cache.ixname
	time -f %E ./lookup -c getspnam -f $(TESTDATA)/rand_spnames
	@echo Binary shadow lookups
	./scripts/index.sh $(TESTDATA)/shadow.cache 1 $(TESTDATA)/shadow.cache.ixname
	time -f %E ./lookup -c getspnam -f $(TESTDATA)/rand_spnames
	gcov --all-blocks --branch-probabilities --branch-counts --function-summaries --unconditional-branches ./lookup

gen_getent: gen_getent.o $(LIBNSSCACHE)


.PHONY: test_getent
test_getent: getent_data gen_getent nss_cache.c
	./gen_getent
	diff $(TESTDATA)/passwd.cache $(TESTDATA)/passwd.cache.out
	diff $(TESTDATA)/group.cache $(TESTDATA)/group.cache.out
	diff $(TESTDATA)/shadow.cache $(TESTDATA)/shadow.cache.out
	gcov --all-blocks --branch-probabilities --branch-counts --function-summaries --unconditional-branches ./gen_getent

.PHONY: lookup_data
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

.PHONY: getent_data
getent_data: $(TESTDATA)
	./scripts/gentestdata.sh $(TESTDATA)

last_pw_errno_test: test/last_pw_errno_test.c

$(TESTDATA):
	mkdir -p $(TESTDATA)

$(LIBRARY): LDFLAGS += -shared $(LD_SONAME)
$(LIBRARY): $(LIBNSSCACHE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(LIBRARY) $+

$(SONAME): $(LIBRARY)
	ln -sf $(LIBRARY) $(SONAME)
	ln -sf $(LIBRARY) $(SONAME).0

.PHONY: install
install: $(SONAME)
	install -d $(LIBDIR)
	install $(LIBRARY) $(LIBDIR)
	install $(SONAME) $(LIBDIR)
	install $(SONAME).0 $(LIBDIR)

.PHONY: clean
clean:
	rm -f $(LIBRARY)* *.o compat/*.o *.gcda *.gcno compat/*.gcda compat/*.gcno lookup gen_getent last_pw_errno_test 

.PHONY: veryclean
veryclean: clean
	rm -rf $(TESTDATA)

.PHONY: distclean
distclean: veryclean
	rm -f *~ \#*

.PHONY: dist
dist:
	rm -rf libnss-cache-$(VERSION) libnss-cache-$(VERSION).tar libnss-cache-$(VERSION).tar.gz
	mkdir libnss-cache-$(VERSION)
	cp $(SOURCES) libnss-cache-$(VERSION)
	tar cf libnss-cache-$(VERSION).tar libnss-cache-$(VERSION)
	gzip -9 libnss-cache-$(VERSION).tar
	rm -rf libnss-cache-$(VERSION)
