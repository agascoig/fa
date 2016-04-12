library(Rcpp)
library(shiny)
sourceCpp("predict.cpp")
phrase<-'the quick brown'
initR()

prediction<-predictR(phrase)
