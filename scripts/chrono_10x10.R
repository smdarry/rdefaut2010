count <- 795

A <- matrix(scan("../output/chrono_10x10.csv", n = 3 * count, sep = ','), count, 3)

A <- t(A)

plot(A[1,], type="l", col="blue", xlab="Image no.", ylab="Intensite", main="Chronogramme du pixel [10,10]")
lines(A[2,], type="l", col="green")
lines(A[3,], type="l", col="red")

x = 1:count

mean_blue = array(mean(A[1,]), c(1, count))
mean_green = array(mean(A[2,]), c(1, count))
mean_red = array(mean(A[3,]), c(1, count))

## lines(x, mean_blue, col="blue")
## lines(x, mean_green, col="green")
## lines(x, mean_red, col="red")
