LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
LIBDIR ?= libs/x86_64
else
LIBDIR ?= libs/x86
endif

CFLAGS ?= -g -Wall -Werror -Wshadow -O2 -pipe -std=gnu11
LDLIBS=$(LIBDIR)/libsepol.a 

all: sepolicy-inject

sepolicy-inject: sepolicy-inject.c
