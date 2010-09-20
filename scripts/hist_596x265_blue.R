A <- matrix(scan("output/chrono_596x265.mat", n = 3 * 795), 3, 795)
hist(A[1,], xlim=c(0,255), xlab="Intensité", ylab="Fréquence", main="Histogramme du pixel [596,265]", col="blue")