CFLAGS = -Wall -O2 -std=gnu99 -g
CFLAGS += -I./include
LDFLAGS = -lm -lcrypto
OBJDIR = obj

all: bench

obj/%.o: src/%.c include/%.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

bench: obj/quotient-filter.o src/bench.c
	$(CC) $(CFLAGS) -o $@ $^

bench2: obj/quotient-filter.o obj/quotient-filter-file.o obj/hashutil.o \
		obj/partitioned_counter.o obj/gqf.o obj/gqf_file.o src/bench2.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

plot-mem:
	gnuplot script/insert-mem.gp
	gnuplot script/lookup-mem.gp
plot-disk:
	gnuplot script/insert-disk.gp
	gnuplot script/lookup-disk.gp

clean:
	$(RM) -rf obj/ bench bench2 bench3 data.qf data.cqf *.png \
			qf_*_benchmark cqf_*_benchmark