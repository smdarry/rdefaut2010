cvs_file = paste("output/chrono_", row, "x", col, ".csv", sep="");
title = paste("Chronogramme du pixel [", row, ", ", col, "]", sep="");

A <- matrix(scan(cvs_file, n = 3 * count, sep = ','), count, 3);
A <- t(A);

plot(A[1,], type="l", col="blue", xlab="Image no.", ylab="Intensite", main=title)
lines(A[2,], type="l", col="green")
lines(A[3,], type="l", col="red")

x = 1:count

means = c(mean(A[1,]), mean(A[2,]), mean(A[3,]))
sds = c(sd(A[1,]), sd(A[2,]), sd(A[3,]))


## Mean lines
mean_blue = array(means[1], c(1, count))
mean_green = array(means[2], c(1, count))
mean_red = array(means[3], c(1, count))

lines(x, mean_blue, col="blue", lty=1)
lines(x, mean_green, col="green", lty=1)
lines(x, mean_red, col="red", lty=1)


## Standard deviation lines
high_blue = array(means[1] + sds[1], c(1, count))
high_green = array(means[2] + sds[2], c(1, count))
high_red = array(means[3] + sds[3], c(1, count))

lines(x, high_blue, col="blue", lty=3)
lines(x, high_green, col="green", lty=3)
lines(x, high_red, col="red", lty=3)

low_blue = array(means[1] - sds[1], c(1, count))
low_green = array(means[2] - sds[2], c(1, count))
low_red = array(means[3] - sds[3], c(1, count))

lines(x, low_blue, col="blue", lty=3)
lines(x, low_green, col="green", lty=3)
lines(x, low_red, col="red", lty=3)


## Legends
label_blue = paste("Mu = ", format(means[1], digits=4), sep="")
label_green = paste("Mu = ", format(means[2], digits=4), sep="")
label_red = paste("Mu = ", format(means[3], digits=4), sep="")

sd_label_blue = paste("3*(Sigma = ", format(sds[1], digits=4), ")", sep="")
sd_label_green = paste("3*(Sigma = ", format(sds[2], digits=4), ")", sep="")
sd_label_red = paste("3*(Sigma = ", format(sds[3], digits=4), ")", sep="")

legend(x=1, y=255, legend=c(label_blue, label_green, label_red), lty=c(1, 1, 1), col=c("blue", "green", "red"))
legend(x=1, y=200, legend=c(sd_label_blue, sd_label_green, sd_label_red), lty=c(3, 3, 3), col=c("blue", "green", "red"))
