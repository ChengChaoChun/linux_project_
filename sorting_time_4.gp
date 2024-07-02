set terminal pngcairo
set output 'sorting_time_4.png'
set title "Sorting Time Comparison(xorshift128+)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:8000000]
set key left

plot "sort_time_4.dat" using 1:2 title "timsort" with point, \
     "sort_time_4.dat" using 1:3 title "linuxsort" with point, \
     "sort_time_4.dat" using 1:4 title "pdqsort" with point
