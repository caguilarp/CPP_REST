Steps

1. Download and install casablanca 

2. To add more procedures modify the file WebMethods.cpp -> initMethods function
  
    Use MethodExecution->CreateMethod in order to support more stored procedures

3. Init jsonservice App

    - Compile and Run with these args 

    [connection string] [service address string]

    example

    "DRIVER={SQL Server};Server=127.0.0.1,1433;Database=pcstats;UID=sa;PWD=elmismo;" http://localhost:1984
    
4. Test and Result
    
    Json Input Example
    
    {
      "method": "Procedure1",
      "parameters": {
        "param1": "<param_value>",
        "param2": "<param_value>",
        "param3": "<param_value>",
        "param4": "<param_value>",
        "param5": "<param_value>"
      }
    }
    
    Json Output Example
    
    {
      "Data": [ <Rows> ],
      "Header": [ <Column Names> ],
      "Result":  <Return Value>
    }


    
    
