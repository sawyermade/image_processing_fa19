# Image Processing Grad USF Fall 2019 HW3
## Processes:


## Parameters for Processes: WS is odd number that will be squared
histgs X Y Sx Sy a b

histgsbl X Y Sx Sy a b c d

othgs X Y Sx Sy

othbr X Y Sx Sy 

othfg X Y Sx Sy 

histgsoth X Y Sx Sy 

histhsii X Y Sx Sy 

histhsihi X Y Sx Sy Hmin Hmax

## Input File Format:
input/file output/file #Processes ProcessName Parameters
### Example:
```
          Input                  Output                  #P    P       X  Y   Sx  Sy
sample-images/baboon.ppm output-images/baboon_roi12ds.ppm 1 histgsoth 50 100 150 300
```

## Compile and Run:
Output images written in output-images/ directory after execution
```
# Clone from github if needed
git clone https://github.com/sawyermade/image_processing_fa19.git 
cd image_processing_fa19/hw2
make
./hw parameters.txt
```

## Parameters file
```
cat parameters.txt
```
