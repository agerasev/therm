#!/bin/sh

gnuplot -e "splot 'out.txt' matrix with lines palette; pause -1"
