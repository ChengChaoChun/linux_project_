set terminal pngcairo
set output 'sorting_time_3.png'
set title "Sorting Time Comparison(xoroshiro128+)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:20000000]
set key left

plot "sort_time_3.dat" using 1:2 title "Timsort Time" with point, \
     "sort_time_3.dat" using 1:3 title "linuxsort Time" with point, \
     "sort_time_3.dat" using 1:4 title "pdqsort Time" with point
