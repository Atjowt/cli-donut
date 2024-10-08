.PHONY: all run clean

CC := clang
CFLAGS := -std=c99 -pedantic -O3 -Wall -Wextra -lm

all: donut

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

run: donut
	./$<

clean:
	rm -f donut
