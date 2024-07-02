set terminal pngcairo
set output 'sorting_time_2.png'
set title "Sorting Time Comparison(Fisher Yates shuffling)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:10000000]
set key left

plot "sort_time_2.dat" using 1:2 title "timsort Time" with point, \
     "sort_time_2.dat" using 1:3 title "linuxsort Time" with point, \
     "sort_time_2.dat" using 1:4 title "pdqsort Time" with point
