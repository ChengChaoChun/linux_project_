set terminal pngcairo
set output 'ktime_3.png'
set title "Kernel Sorting Time Comparison(xoroshiro128+)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:20000000]
set key left

plot "ktime_3.dat" using 1:2 title "timsort Time" with point, \
     "ktime_3.dat" using 1:3 title "linuxsort Time" with point, \
     "ktime_3.dat" using 1:4 title "pdqsort Time" with point
