library(shiny)
library(ggplot2)
library(readxl)
library(mapdata)
library(lubridate)
library(dplyr)

load("/Users/clairez/Desktop/app/labdata20SU21.RData")
state <- map_data("state")
table<-labdata20SU21[,c("STATENAME",
                        "STATE",
                        "DRINKINGNAME",
                        "DRUGSNAME",
                        "REST_USENAME",
                        "LONGITUD",
                        "LATITUDE"
                        )]
byState<-read_excel("/Users/clairez/Desktop/app/byState.xlsx")
colnames(labdata20SU21)[2]<-"group"
colnames(state)[5]<-"STATENAME"
labdata20SU21[,"STATENAME"]<-tolower(labdata20SU21[,"STATENAME"])
drug<-labdata20SU21[,c("STATENAME","group","DRUGSNAME","LONGITUD","LATITUDE")]
drug<-filter(drug, drug$DRUGSNAME == "Yes (drugs involved)")
colnames(drug)[3]<-"reason"
drink<-labdata20SU21[,c("STATENAME","group","DRINKINGNAME","LONGITUD","LATITUDE")]
drink<-filter(drink, drink$DRINKINGNAME == "Yes (Alcohol Involved)")
colnames(drink)[3]<-"reason"
belt<-labdata20SU21[,c("STATENAME","group","REST_USENAME","DEATHS","LONGITUD","LATITUDE")]
belt<-filter(belt, belt$REST_USENAME == "None Used" & belt$DEATHS != 0)
colnames(belt)[3]<-"reason"

# Define UI for application 
ui <- fluidPage(
    titlePanel("US Highway Fatality Barplots and Maps"),
    h2("Introduction"),
    p("This is an app with which users can check the number or percentage of highway fatality cases by state.
      Users may also choose which category of data they want to see. The three maps illustrate the spread of 
      drugs/alcohol/no-belt involved highway fatality cases across the nation."),
    p("The aim of this app is to inform users how and how much drugs/alcohol/no belt use influence highway safety
      in the US."),
    sidebarLayout(
        sidebarPanel(
            selectInput(
                inputId = "value",
                label = "select a category to plot",
                choices = c("drugs",
                            "drugs_percent",
                            "alcohol",
                            "alcohol_percent",
                            "no_belt",
                            "no_belt_percent"
                )
            ),
            selectInput(
              inputId = "type",
              label = "select a map to view",
              choices = c("drugs",
                          "alcohol",
                          "no_belt"
              )
            ),
        ),
    mainPanel( 
        #this will create a space for us to display our plots and maps
      h3("Barplots of Number and Percentage of Cases"),
      p("This is an interactive barplot. You may click on the choices and view the barplot of the selected category.
        The choices also include the percentages of each type by state. By looking at the percentage barplots, you can get how
        much drugs/alcohol/no belt use affect US highway fatality in each state."),
      plotOutput(outputId = "barplot"),
      h3("Maps of Drug/Alcohol/No-Belt-Used Cases in US"),
      p("The following three maps show the distribution of drug/alcohol/no-belt-use involved cases across the nation.
        You may click on the choices to see the different distributions. The distributions show how the states and areas
        are influenced by drugs/alcohol/no belt use."),
      plotOutput("m_map")
)))

# Define server logic 
server <- function(input,output) {
  datasetInput <- reactive({
    if (input$value == "drugs") {
      dataset<-byState[,c(1,4)]
    }
    else if (input$value == "drugs_percent") {
      dataset <- byState[,c(1,8)]
    }else if (input$value == "alcohol") {
      dataset <- byState[,c(1,3)]
    }else if (input$value == "alcohol_percent") {
      dataset <- byState[,c(1,7)]
    }else if (input$value == "no_belt") {
      dataset <- byState[,c(1,5)]
    }else if (input$value == "no_belt_percent") {
      dataset <- byState[,c(1,9)]
    }
    return(dataset)
  })
  
  color <- reactive({
    if (input$value == "drugs" || input$value == "drugs_percent") {
      color<-"red"
    }
    else if (input$value == "alcohol" ||input$value == "alcohol_percent") {
      color <- "skyblue"
    }
    else if (input$value == "no_belt" ||input$value == "no_belt_percent") {
      color <- "violet"
    }
    return(color)
  })
  
    
    output$barplot <-renderPlot({
        ggplot(datasetInput(), aes_string(x="State",y=input$value))  +
            stat_summary(fun = sum, geom = "bar",colour=color(),fill="#56B4E9") +
            geom_bar(stat="identity") +
            labs(title=input$value, y ="proportion") +
            theme_classic() +
            theme(plot.title = element_text( hjust = 0.5)) +
            theme(axis.text.x = element_text(angle = 90))
})
    
    datasetInput2 <- reactive({
      if (input$type == "drugs") {
        dataset<-drug
      }
      else if (input$type == "alcohol") {
        dataset <- drink
      }else if (input$type == "no_belt") {
        dataset <- belt
      }
      return(dataset)
    })
    
    color2 <- reactive({
      if (input$type == "drugs") {
        color <- "red"
      }
      else if (input$type == "alcohol") {
        color <- "blue"
      }else if (input$type == "no_belt") {
        color <- "purple"
      }
      return(color)
    })
    
    output$m_map <-
      renderPlot({
        g <- ggplot(map_data("state"), 
                    aes(x = long, 
                        y = lat, 
                        group = group)) +
          geom_polygon(fill="gold", 
                       colour = "black") 
        g +  geom_point(inherit.aes = FALSE, 
                        data = datasetInput2(), 
                        aes(fill = reason, 
                          x = LONGITUD,
                          y = LATITUDE),
                        color = color2()) +
          xlim(c(-125,-65))+
          ylim(c(25,50))+
          labs( title = "Distribution in the US",
                x = "Longitude", 
                y = "Latitude") +
          theme_classic()
      })
    
}

# Run the application 
shinyApp(ui = ui, server = server)
