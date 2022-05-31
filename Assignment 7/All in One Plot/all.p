
set terminal png
set output "all.png"

set title "Page replacement using various policies"

set key right center

set xlabel "frms in memory"
set ylabel "hit ratio"

set xrange [0:100]
set yrange [0:1]

plot "random.dat" u 1:2 w linespoints title "random", \
	"optimal.dat" u 1:2 w linespoints title "optimal", \
	"lru.dat" u 1:2 w linespoints title "lru", \
	"clock.dat" u 1:2 w linespoints title "clock"

