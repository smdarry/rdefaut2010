A <- matrix(scan("../output/chrono_10x10.mat", n = 3 * 795), 3, 795)
hist(A[2,], xlim=c(0,255), xlab="Intensite", ylab="Frequence", main="Histogramme du pixel [10,10]", col="green")
