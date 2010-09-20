A <- matrix(scan("output/chrono_217x137.mat", n = 3 * 795), 3, 795)

plot(A[1,], type="l", col="blue", xlab="Image no.", ylab="Intensité", main="Chronogramme du pixel [217,137]")
lines(A[2,], type="l", col="green")
lines(A[3,], type="l", col="red")

x = 1:795

mean_blue = array(mean(A[1,]), c(1, 795))
mean_green = array(mean(A[2,]), c(1, 795))
mean_red = array(mean(A[3,]), c(1, 795))

lines(x, mean_blue, col="blue")
lines(x, mean_green, col="green")
lines(x, mean_red, col="red")