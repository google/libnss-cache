CC=gcc
#CFLAGS=-Wall -Wstrict-prototypes -Werror -fPIC -DDEBUG -g -O0
CFLAGS=-Wall -Wstrict-prototypes -Werror -fPIC
LIBRARY=libnss_cache.so.2.0
LINKS=libnss_cache.so.2 libnss_cache.so
PREFIX=/usr
BUILD=.libs
BUILD64=.libs64
BUILD32=.libs32
LIBDIR=$(PREFIX)/lib
LIBDIR64=$(PREFIX)/lib64
LIBDIR32=$(PREFIX)/lib32

default:
	@echo Unknown Architecture

build_i386: nss_cache64

build_amd64: nss_cache32

install_i386: install64

install_amd64: install32

nss_cache:
	[ -d $(BUILD) ] || mkdir $(BUILD)
	$(CC) $(CFLAGS) -c nss_cache.c -o $(BUILD)/nss_cache.o
	$(CC) -shared -o $(BUILD)/$(LIBRARY) $(BUILD)/nss_cache.o

nss_cache64: nss_cache
	[ -d $(BUILD64) ] || mkdir $(BUILD64)
	$(CC) $(CFLAGS) -m64 -c nss_cache.c -o $(BUILD64)/nss_cache.o
	$(CC) -m64 -shared -o $(BUILD64)/$(LIBRARY) $(BUILD64)/nss_cache.o

nss_cache32: nss_cache
	[ -d $(BUILD32) ] || mkdir $(BUILD32)
	$(CC) $(CFLAGS) -m32 -c nss_cache.c -o $(BUILD32)/nss_cache.o
	$(CC) -m32 -shared -o $(BUILD32)/$(LIBRARY) $(BUILD32)/nss_cache.o

clean:
	rm -rf $(BUILD)
	rm -rf $(BUILD32)
	rm -rf $(BUILD64)

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
