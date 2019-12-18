reset
set key left
set xlabel 'Load Factor'
set ylabel 'Million Operations per Second'
set title 'In-memory Insertion Performance Comparison'
set terminal png font "Times_New_Roman,12"
set output 'insert-mem.png'
set xtics 0,20,100
set ytics 0,5,50
set grid

plot [0:100][:50] 'qf_mem_benchmark' using 1:2 with linespoints \
                  linewidth 2 title 'QF', \
                  'cqf_mem_benchmark' using 1:2 with linespoints \
                  linewidth 2 title 'CQF'
