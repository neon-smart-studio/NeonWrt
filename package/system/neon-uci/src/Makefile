CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Wformat=2 -Wshadow -Wpointer-arith
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -luci

all: neon-uci

neon-uci: neon-uci.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

clean:
	rm -f neon-uci

.PHONY: all clean
