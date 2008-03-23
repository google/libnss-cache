CC=gcc
#CFLAGS=-Wall -fPIC -DDEBUG -g -O0
CFLAGS=-Wall -fPIC
LIBRARY=libnss_cache.so.2.0
LINKS=libnss_cache.so.2 libnss_cache.so
PREFIX=/usr
BUILD=.libs
LIBDIR=$(PREFIX)/lib

nss_cache:
	[ -d $(BUILD) ] || mkdir $(BUILD)
	$(CC) $(CFLAGS) -c nss_cache.c -o $(BUILD)/nss_cache.o
	$(CC) -shared -o $(BUILD)/$(LIBRARY) $(BUILD)/nss_cache.o

clean:
	rm -rf $(BUILD)

install:
	[ -d $(LIBDIR) ] || install -d $(LIBDIR)
	install $(BUILD)/$(LIBRARY) $(LIBDIR)
	cd $(LIBDIR); for link in $(LINKS); do ln -sf $(LIBRARY) $$link ; done
