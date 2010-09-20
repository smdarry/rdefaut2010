A <- matrix(scan("../output/chrono_10x10.csv", n = 3 * 795, sep = ','), 3, 795)
hist(A[1,], xlim=c(0,255), xlab="Intensite", ylab="Frequence", main="Histogramme du pixel [10,10]", col="blue")
