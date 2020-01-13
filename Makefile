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
test: obj/quotient-filter.o obj/quotient-filter-file.o obj/hashutil.o \
		obj/partitioned_counter.o obj/gqf.o obj/gqf_file.o src/test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

plot-mem:
	gnuplot script/insert-mem.gp
	gnuplot script/lookup-mem.gp
plot-disk:
	gnuplot script/insert-disk.gp
	gnuplot script/lookup-disk.gp
plot-space:
	python3 script/space-usage.py > space-usage.txt
	gnuplot script/space-usage.gp
	rm space-usage.txt

clean:
	$(RM) -rf obj/ bench bench2 test data.qf data.cqf *.png \
			qf_*_benchmark cqf_*_benchmark \
			space-analysis.png
