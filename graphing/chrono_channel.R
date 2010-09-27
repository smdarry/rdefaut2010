cvs_file = paste("output/chrono_", row, "x", col, ".csv", sep="");
title = paste("Chronogramme du pixel [", row, ", ", col, "]", sep="");
colors = c("blue", "green", "red")

A <- matrix(scan(cvs_file, n = 3 * count, sep = ','), count, 3);
A <- t(A);

plot(A[channel,], type="l", col=colors[channel], xlab="Image no.", ylab="Intensite", main=title)

x = 1:count

## Mean lines
meand = array(mean(A[channel,]), c(1, count))
lines(x, meand, lty=1)

## Standard deviation lines
high = array(meand + sd(A[channel,]), c(1, count))
lines(x, high, lty=3)

low = array(meand - sd(A[channel,]), c(1, count))
lines(x, low, lty=3)

## Legends
label = paste("Mu = ", format(mean(A[channel,]), digits=4), sep="")
sd_label = paste("3*(Sigma = ", format(sd(A[channel,]), digits=4), ")", sep="")

legend(x=1, y=140, legend=c(label, sd_label), lty=c(1,2), col="black")
