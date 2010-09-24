csv_file = paste("output/chrono_", row, "x", col, ".csv", sep="");
title = paste("Histogramme temporel du pixel [", row, ", ", col, "]", sep="");
colors = c("blue", "green", "red")

A <- matrix(scan(csv_file, n=3 * count, sep=','), 3, count)
hist(A[channel,], xlim=c(0,255), xlab="Intensite", ylab="Frequence", main=title, col=colors[channel])
