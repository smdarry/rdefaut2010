A <- matrix(scan("output/chrono_217x137.mat", n = 3 * 795), 3, 795)
hist(A[1,], xlim=c(0, 255), xlab="Intensité", ylab="Fréquence", main="Histogramme du pixel [217,137]", col="blue")