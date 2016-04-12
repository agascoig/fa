library(shiny)
library(Rcpp)
sourceCpp("predict.cpp")
initR()

shinyServer(
        function(input, output, session) {
                output$text1<-reactive({
                        predictR(input$text)
                })
        
        }
)
