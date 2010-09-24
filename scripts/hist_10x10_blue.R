cvs_file = paste("output/hist_", row, "x", column, ".csv", sep="");
title = paste("Histogramme temporel du pixel [", row, ", ", column, "]", sep="");
colors = c("blue", "green", "red")

A <- matrix(scan(csv_file, n=3 * count, sep=','), 3, count)
hist(A[1,], xlim=c(0,255), xlab="Intensite", ylab="Frequence", main=title, col=colors[channel])
