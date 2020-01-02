set title ""
set ylabel '-log_{10}(false positive rate)'
set xlabel 'Bits/element'
set terminal png font " Times_New_Roman,12 "
set output "space-analysis.png"
set datafile separator ","
set key left
plot [4:32][:] 'space-usage.txt' i 0 u 1:2 w lines title "Bloom Filter",\
    "space-usage.txt" i 1 u 4:5 w lines title "Quotient Filter (3 bits, alpha=0.75)",\
    "space-usage.txt" i 2 u 4:5 w lines title "Quotient Filter (3 bits, alpha=0.9)"