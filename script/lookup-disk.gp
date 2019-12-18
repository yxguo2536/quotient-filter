reset
set key left
set xlabel 'Load Factor'
set ylabel 'Thousand Operations per Second'
set title 'In-disk Lookup Performance Comparison'
set terminal png font "Times_New_Roman,12"
set output 'lookup-disk.png'
set xtics 0,20,100
set ytics 0,1000,10000
set grid

plot [0:100][:10000] 'qf_disk_benchmark' using 1:3 with linespoints \
				     linewidth 2 title 'QF', \
     			     'cqf_disk_benchmark' using 1:3 with linespoints \
     			     linewidth 2 title 'CQF'
