.PHONY: all run clean

CC := clang
CFLAGS := -std=c99 -pedantic -Wall -Wextra -lm
CFLAGS += -Ofast
# CFLAGS += -g

all: donut

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

run: donut
	./$<

clean:
	rm -f donut
