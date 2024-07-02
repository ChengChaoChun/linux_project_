set terminal pngcairo
set output 'ktime_4.png'
set title "Kernel Sorting Time Comparison(xorshift128+)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:8000000]
set key left

plot "ktime_4.dat" using 1:2 title "timsort" with point, \
     "ktime_4.dat" using 1:3 title "linuxsort" with point, \
     "ktime_4.dat" using 1:4 title "pdqsort" with point 
