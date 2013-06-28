PREFIX ?= $(DESTDIR)/usr
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib/x86_64-linux-gnu

CFLAGS ?= -g -Wall -Werror -Wshadow -O2 -pipe -fno-strict-aliasing
LDLIBS=$(LIBDIR)/libsepol.a 

all: sepolicy-inject

sepolicy-inject: sepolicy-inject.c
