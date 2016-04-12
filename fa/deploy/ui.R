library(shiny)

shinyUI(
        pageWithSidebar(
                # Application title
                titlePanel("Coursera Data Science Capstone: Text Prediction"),
                sidebarPanel(
                        textInput('text',label="Enter Text Here:"),
                        submitButton("Predict Next Word")
                ),
                mainPanel(
                        p("Copyright 2016 Alex Gascoigne (agascoig@ieee.org)"),
                        hr(),
                        p("Given 1 to 6 words separated by spaces, predict the next word."),
                        hr(),
                        textOutput("text1")
                )
        )
)