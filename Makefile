CFLAGS = -Wall -O2 -std=gnu99 -g
CFLAGS += -I./include

all: bench

obj/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

bench: obj/quotient-filter.o src/bench.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) obj/*.o obj/*.so bench


