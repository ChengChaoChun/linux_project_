set terminal pngcairo
set output 'ktime_1.png'
set title "Kernel Sorting Time Comparison(descending order)"
set xlabel "Number of Elements"
set ylabel "Time (ns)"
set grid
set style data linespoints
set yrange [0:8000000]
set key left

plot "ktime_1.dat" using 1:2 title "timsort" with point, \
     "ktime_1.dat" using 1:3 title "linuxsort" with point, \
     "ktime_1.dat" using 1:4 title "pdqsort" with point 
