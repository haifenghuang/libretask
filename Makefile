INSTALL=install
LIBS = -lhiredis -ljson
CFLAGS = -c -g -D_GNU_SOURCE -std=gnu99 -Wall -fPIC -fstack-protector-all --shared
CFLAGS_BIN = -g -D_GNU_SOURCE -std=gnu99 -Wall -fstack-protector-all


PREFIX = /usr
LIBDIR_32 = $(PREFIX)/lib
LIBDIR_64 = $(PREFIX)/lib64

ARCH = $(shell getconf LONG_BIT)
LIBDIR = $(LIBDIR_$(ARCH))
TARGET = libretask.so
SOURCES = retask.c
HEADERDIR = $(PREFIX)/include

all:
	gcc $(CFLAGS) -I/usr/include/hiredis/ $(LIBS) $(SOURCES) -o $(TARGET)
	gcc -g dequeue_example.c -o dequeue_example -lhiredis $(CFLAGS_BIN) -I/usr/include/hiredis/ -L. $(LIBS) -lretask
	#gcc -g dequeue_example2.c -o dequeue_example2 -lhiredis -ljson -lretask -I/usr/include/hiredis/ -L./ -fstack-protector-all
	#gcc -g enqueue_example.c -o enqueue_example -lhiredis -ljson -lretask -I/usr/include/hiredis/ -L./ -fstack-protector-all
	#gcc -g dequeue2.c -o dequeue2 -lhiredis -ljson -lretask -I/usr/include/hiredis/ -L./ -fstack-protector-all
	#gcc -g enqueue2.c -o enqueue2 -lhiredis -ljson -lretask -I/usr/include/hiredis/ -L./ -fstack-protector-all

.PHONY: install
install:
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) libretask.so $(LIBDIR)/
	$(INSTALL) -d -m 755 $(HEADERDIR)
	$(INSTALL) retask.h $(HEADERDIR)/


.PHONY: clean
clean:
	rm -rf $(TARGET)