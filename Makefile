CFLAGS = -Wall -O2 -std=gnu99 -g
CFLAGS += -I./include
LDFLAGS = -lm -lcrypto

all: bench

obj/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

bench: obj/quotient-filter.o src/bench.c
	$(CC) $(CFLAGS) -o $@ $^

bench2: obj/quotient-filter.o obj/hashutil.o obj/partitioned_counter.o obj/gqf.o src/bench2.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) obj/*.o obj/*.so bench bench2


