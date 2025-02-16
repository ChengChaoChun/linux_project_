set terminal pngcairo
set output 'ktime_2.png'
set title "Kernel Sorting Time Comparison(Fisher Yates shuffling)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:10000000]
set key left

plot "ktime_2.dat" using 1:2 title "timsort Time" with point, \
     "ktime_2.dat" using 1:3 title "linuxsort Time" with point, \
     "ktime_2.dat" using 1:4 title "pdqsort Time" with point
