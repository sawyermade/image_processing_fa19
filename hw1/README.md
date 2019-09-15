# Image Processing Grad USF Fall 2019 HW1
## Processes:
bin = grayscale binarization
binc = color binarization
2Dsmooth = 2D smoothing

## Parameters for Processes: WS is odd number that will be squared
bin X Y Sx Sy T
binc X Y Sx Sy TC Cr Cg Cb
2Dsmooth X Y Sx Sy WS

## Input File Format:
input/file output/file #Processes ProcessName Parameters
### Example:
```
          Input                  Output                  #P    P     X   Y   Sx  Sy WS
sample-images/baboon.ppm output-images/baboon_roi12ds.ppm 1 2Dsmooth 50 100 150 300 5
```

## Compile and Run:
Output images written in output-images/ directory after execution
```
git clone https://github.com/sawyermade/image_processing_fa19.git 
cd image_processing_fa19/hw1
make
./hw parameters.txt
```

## Parameters file
```
cat parameters.txt
```
